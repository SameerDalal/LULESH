#include "Backtrace.h"
#include "ContextNode.h"
#include <string>
#include <iostream>
#include <fstream>
//#define UNW_LOCAL_ONLY
#include <libunwind.h>
#include <cxxabi.h>
#include <cstdarg>
#include <algorithm>

// Assigning functions to be executed before and after main()
//void __attribute__((constructor)) bootstrap();
//void __attribute__((destructor)) finalize();

//The node for currently traced function
ContextNode* topNode;

//all the traced nodes
std::vector<ContextNode*> nodes;
static void write_to_dot();

static void print_node_info_brief(ContextNode* node) {
    //std::cout << "node:location:funcName:" << node << ":" << (void*)ip <<":" << funcName << std::endl;
    std::cout << node << ":" << node->getReturnAddress() <<":" << node->getFunctionName();
    int num_args = node->getArguments().size();
    if (num_args > 0) {
        for(auto & arg: node->getArguments()) {
            std::cout << arg << ",";
        }
    }
    std::cout << std::endl;
}

void bootstrap() {
    //this is used before main is called, we could just call trace_func_call, but only the following
    //are useful, thus we will not call trace_func_call here
    unw_context_t context;
    unw_cursor_t cursor;
    unw_word_t ip;
    unw_getcontext(&context);
    unw_init_local(&cursor, &context);
    unw_step(&cursor);
    unw_get_reg(&cursor, UNW_REG_IP, &ip); //The address/location of the call
    topNode = new ContextNode("main", (void*) ip);
    topNode->addCallCount(1);
    topNode->setParentNode(nullptr);
    //print_node_info_brief(topNode);
    nodes.push_back(topNode);
}

void finalize() {
    trace_func_call_end(); //to finish main function
    write_to_dot();
}

/**
 * ... is a pair of formal_parameter:actual_parameter
 * 
 * 
 */
void trace_func_call(std::string funcName, int num_args, ...) {
    unw_context_t context;
    unw_cursor_t cursor;
    unw_word_t ip;
    unw_getcontext(&context);
    unw_init_local(&cursor, &context);
    unw_step(&cursor);
    unw_get_reg(&cursor, UNW_REG_IP, &ip); //The address/location of the call

    //search using ip the children nodes of the current topNode to see whether this function is called before at this location 
    //if found, increae only call count. If not found, this is the first time, create the node and add as child node
    for (auto & node : topNode->getChildren()) {
        if (node->getReturnAddress() == (void*)ip) {
            node->addCallCount(1);
            topNode = node;
            return;
        }
    }

    ContextNode* node = new ContextNode(funcName, (void*)ip);

    //store the argument information 
    char* value;
    va_list argp;
    va_start(argp, num_args);
    int i;
    for (i=0; i<num_args; i++) {
        value = va_arg(argp, char*);
        node->addArgument(value);
    }
    va_end(argp);
    
    node->addCallCount(1);
    node->setParentNode(topNode);
    nodes.push_back(node);
    //print_node_info_brief(node);
    topNode->addChild(node);
    topNode = node;
}

void trace_func_call_end() {
    topNode = topNode->getParentNode();
}

//dump the CCT tree to dot files 
void write_to_dot() {
    std::ofstream dotFileWrite("tree.dot");
    dotFileWrite << "digraph ContextTree {" << std::endl;
    dotFileWrite << "rankdir=\"LR\"" << std::endl;
    for (auto & node : nodes) {
        //print_node_info_brief(node);
        dotFileWrite << node->getFunctionName() << node <<
            "[label=\"" << node->getFunctionName() <<
            "\\n Call Location: 0x" << std::hex << node->getReturnAddress() <<
            "\"];" << std::endl;

        for (auto &child  : node->getChildren()) {
            if (child->getArguments().size() == 0) {
                dotFileWrite << node->getFunctionName() << node << " -> " << 
                         child->getFunctionName() << child << "[label=\" "; 
                    dotFileWrite << child->getCallCount() << "x";
                    dotFileWrite << "\"];" << std::endl;
            } else {
                int i = 0;
                for(auto & param: child->getArguments()) {
                    dotFileWrite << node->getFunctionName() << node << " -> " << 
                         child->getFunctionName() << child << "[style=bold, label=\" "; 
                    dotFileWrite << param << "," << child->getCallCount() << "x";
                    dotFileWrite << "\"];" << std::endl;
                }
            } 
        }
    }
    dotFileWrite << "}" << std::endl;
    dotFileWrite.close();
}

//memory cleanup for unused nodes