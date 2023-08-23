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
#include <cmath>

#include <cstdlib>
#include <ctime>
#include <sstream>
#include <iomanip>

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
    trace_end(); //to finish main function
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
    node->setParallelBlock(false);
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

    setColor(node);
    std::cout << "Function Arg: " << node->getArguments().size() << std::endl;
    std::cout << "Function Color: " << node->getColor().size() << std::endl;

    topNode = node;
}

std::string generateRandomColor() {
    srand(time(nullptr));

    unsigned int randomColor = rand() % 0xFFFFFF;
    std::stringstream ss;
    ss << "#" << std::hex << std::setw(6) << std::setfill('0') << randomColor;

    return ss.str();
}

void setColor(ContextNode* node) {
    srand(time(nullptr)); 
    if(node->getParentNode()->getArguments().size() == 0 || node->getArguments().size() == 0) {
        for(auto i : node->getArguments()) {
            node->setColor("#000000");
        }
    } else {
        for(auto param : node->getParentNode()->getArguments()) {
            int i = 0;
            for(auto params : node->getArguments()) {
                if(params == param) {
                    if(node->getParentNode()->getColor()[i] == "#000000") {
                        node->setColor(generateRandomColor());
                    } else {
                        node->setColor(node->getParentNode()->getColor()[i]);
                    }
                } else {
                    node->setColor("#000000");
                }
                i++;
            }
        }
    }
}

void trace_parallel_block(std::string funcName, int num_args, ...) {
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
    node->setParallelBlock(true);
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

    setColor(node);
    std::cout << "Function Arg: " << node->getArguments().size() << std::endl;
    std::cout << "Function Color: " << node->getColor().size() << std::endl;

    topNode = node;
}
 

void trace_end() {
    topNode = topNode->getParentNode();
}


//dump the CCT tree to dot files 
void write_to_dot() {
    std::ofstream dotFileWrite("tree.dot");
    dotFileWrite << "digraph ContextTree {" << std::endl;
    dotFileWrite << "rankdir=\"LR\"" << std::endl;
    dotFileWrite << "node [style=\"filled\", fontname=\"Times-Roman\", fontsize=12, fillcolor=lightblue2, fontcolor=\"#000000\"];" << std::endl;
    dotFileWrite << "edge [color=black, fontname=\"Times-Roman\", fontsize=10];" << std::endl;

    for (auto & node : nodes) {
        //print_node_info_brief(node);
        if(node->getParallelBlock()) {
            dotFileWrite << node->getFunctionName() << node <<
            "[label=\"" << node->getFunctionName() <<
            "\\n Call Location: 0x" << std::hex << node->getReturnAddress() <<
            "\", fillcolor=\"#90EE90\", style=filled, fontcolor=\"#770000\"];" << std::endl;
        } else {
            dotFileWrite << node->getFunctionName() << node <<
            "[label=\"" << node->getFunctionName() <<
            "\\n Call Location: 0x" << std::hex << node->getReturnAddress() <<
            "\"];" << std::endl;
        }

        for (auto &child  : node->getChildren()) {
            if (child->getArguments().size() == 0) {
                dotFileWrite << node->getFunctionName() << node << " -> " << 
                         child->getFunctionName() << child << "[label=\" "; 
                    dotFileWrite << std::to_string(child->getCallCount()) << "x";
                    dotFileWrite << "\", color=\"" << "#00000" << "\"];" << std::endl;
            } else {
                int i = 0;
                for(auto & param: child->getArguments()) {
                    dotFileWrite << node->getFunctionName() << node << " -> " << 
                         child->getFunctionName() << child << "[label=\" "; 
                    dotFileWrite << param << "," << std::to_string(child->getCallCount()) << "x";
                    dotFileWrite << "\", color=\"" << std::hex << child->getColor()[i] << "\"];" << std::endl;
                    i++;
                }
            }
        }
    }
    dotFileWrite << "}" << std::endl;
    dotFileWrite.close();
}

//memory cleanup for unused nodes