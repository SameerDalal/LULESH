#include <iostream>
#include "ContextNode.h"
#include <string>
#include <vector>
#include <libunwind.h>

ContextNode::ContextNode(void* frame_addr, 
                        void* return_addr, 
                        std::string funcName,
                        unw_word_t start_ip,
                        unw_word_t end_ip,
                        unw_word_t lsda,
                        unw_word_t handler,
                        unw_word_t global_pointer,
                        unw_word_t flags,
                        std::vector<std::string> parameters) { 
    
    this->frame_addr = frame_addr;
    this->return_addr = return_addr;
    this->start_ip = start_ip;
    this->end_ip = end_ip;
    this->lsda = lsda;
    this->handler = handler;
    this->global_pointer = global_pointer;
    this->flags = flags;
    this->funcName = funcName;
    this->parameters = parameters;
    this->arguments = {};
    callCount = 0;
 
}

ContextNode::ContextNode(std::string funcName, void* return_addr) {
    
    this->return_addr = return_addr;
    this->funcName = funcName;
    this->arguments = {};
    callCount = 0;
}

ContextNode::~ContextNode() {
    //delete this;
}

void ContextNode::addChild(ContextNode* child) {
    children.push_back(child);
}

void ContextNode::setParentNode(ContextNode* node){
    parent = node;
}

void ContextNode::setArguments(std::vector<std::string> args) {
    for(auto arg : args) {
        arguments.push_back(arg);
    }
}


// getters

std::vector<std::string> &ContextNode::getArguments() {
    return arguments;
}

void ContextNode::addArgument(std::string arg) {
    arguments.push_back(arg);
}

std::string ContextNode::getFunctionName() {
    return funcName;
};

ContextNode* ContextNode::getParentNode() {
    return parent;
}

std::vector<std::string> & ContextNode::getParameters() {
    return parameters;
}

std::vector<ContextNode*> ContextNode::getChildren() {
    return children;
}

void* ContextNode::getFrameAddress() {
    return frame_addr;
}

void* ContextNode::getReturnAddress() {
    return return_addr;
}

unw_word_t ContextNode::getStartIP() const {
    return start_ip;
}

unw_word_t ContextNode::getEndIP() const {
    return end_ip;
}

unw_word_t ContextNode::getLsda() const {
    return lsda;
}

unw_word_t ContextNode::getHandler() const {
    return handler;
}

unw_word_t ContextNode::getGlobalPointer() const {
    return global_pointer;
}

unw_word_t ContextNode::getFlags() const {
    return flags;
}

void ContextNode::addCallCount(int val) {
    callCount+=val;
}

int ContextNode::getCallCount(){ 
    return callCount;
}