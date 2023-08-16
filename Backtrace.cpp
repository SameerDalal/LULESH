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
#include <sstream>

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
    topNode = node;
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
    topNode = node;
}
 
void trace_end() {
    topNode = topNode->getParentNode();
}

//dump the CCT tree to dot files 
void write_to_dot() {
    std::ofstream functionTree("tree_function.dot");
    std::ofstream parallelTree("tree_parallel.dot");

    std::stringstream writeString;

    writeString << "digraph ContextTree {" << std::endl << "rankdir=\"LR\"" << std::endl
                << "node [style=\"filled\", fontname=\"Times-Roman\", fontsize=12, fillcolor=lightblue2, fontcolor=\"#000000\"];" 
                << std::endl << "edge [color=black, fontname=\"Times-Roman\", fontsize=10];" << std::endl;
    
    functionTree << writeString.str();
    parallelTree << writeString.str();

    writeString.str("");

    auto getGradient = [](int value) -> std::string {
        //int red = (int)(255*(1-pow(1.0025,(-0.02*value)))); -- for general functions
        // for lulesh : 
        int redRGBVal = (int)(255*(1-pow(1.9,(-0.02*value))));
        char hexValue[8];
        snprintf(hexValue, sizeof(hexValue), "#%02X%02X%02X", redRGBVal, 0, 0);
        return hexValue;
    };

    for (auto & node : nodes) {
        //print_node_info_brief(node);            
        
        writeString << node->getFunctionName() << node << "[label=\"" << node->getFunctionName() << "\\n Call Location: 0x" << std::hex << node->getReturnAddress();

        if(node->getParallelBlock()) {
            writeString << "\", fillcolor=\"green"; 
        }
        writeString << "\"];" << std::endl;

        parallelTree << writeString.str();
        functionTree << writeString.str();

        writeString.str("");

        for (auto &child  : node->getChildren()) {
            if (child->getArguments().size() == 0) {

                writeString << node->getFunctionName() << node << " -> " << child->getFunctionName() 
                            << child << "[label=\" " << std::to_string(child->getCallCount()) << "x" 
                            << "\", color=\"" << getGradient(child->getCallCount()) << "\"];" << std::endl;

                parallelTree << writeString.str();
                functionTree << writeString.str();

                writeString.str("");
                
            } else {
                int i = 0;
                for(auto & param: child->getArguments()) {
                    if(child->getParallelBlock()) {

                        writeString << node->getFunctionName() << node << " -> " << child->getFunctionName() 
                                    << child << "[label=\" " << std::to_string(child->getCallCount()) << "x" 
                                    << "\", color=\"" << getGradient(child->getCallCount()) << "\"];" << std::endl;
                        
                        functionTree << writeString.str();

                        writeString.str("");

                        for(auto& param : child->getArguments()) {

                            writeString << node->getFunctionName() << node << " -> " << child->getFunctionName() 
                                        << child << "[label=\" " << param << "," << std::to_string(child->getCallCount()) 
                                        << "x" << "\", color=\"" << getGradient(child->getCallCount()) << "\"];" << std::endl;

                            parallelTree << writeString.str();

                            writeString.str("");
                        }
                        break;
                    } else {
                        writeString << node->getFunctionName() << node << " -> " << child->getFunctionName() 
                                    << child << "[label=\" " << param << "," << std::to_string(child->getCallCount()) 
                                    << "x" << "\", color=\"" << getGradient(child->getCallCount()) << "\"];" << std::endl;
                        
                        functionTree << writeString.str();

                        writeString.str("");
                    }
                }
                writeString << node->getFunctionName() << node << " -> " << child->getFunctionName() 
                            << child << "[label=\" " << std::to_string(child->getCallCount()) << "x" 
                            << "\", color=\"" << getGradient(child->getCallCount()) << "\"];" << std::endl;
                    
                parallelTree << writeString.str();

                writeString.str("");
            }
        }
    }
    
    functionTree << "}" << std::endl;
    functionTree.close();

    parallelTree << "}" << std::endl;
    parallelTree.close();
}

//memory cleanup for unused nodes
