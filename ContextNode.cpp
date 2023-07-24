#include <iostream>
#include "ContextNode.h"
#include <string>
#include <vector>

//private instance variables
//each node will store the symbols function name and memory address
ContextNode* nullNode = new ContextNode("NULL", {}, {}, {}, {}, {}, {}, {});

std::string funcName;
std::vector<std::string> parameters;

unw_word_t start_ip;
unw_word_t end_ip;
unw_word_t lsda;
unw_word_t handler; 
unw_word_t global_pointer;
unw_word_t flags;

ContextNode* parent;
std::vector<ContextNode*> children;
int callCount;


ContextNode::ContextNode(std::string funcName,
                        std::vector<std::string> parameters,
                        unw_word_t start_ip,
                        unw_word_t end_ip,
                        unw_word_t lsda,
                        unw_word_t handler,
                        unw_word_t global_pointer,
                        unw_word_t flags) 
    : funcName(funcName),
      parameters(parameters),
      start_ip(start_ip), 
      end_ip(end_ip),
      lsda(lsda),
      handler(handler),
      global_pointer(global_pointer),
      flags(flags),
      callCount(0)
{

}

/*
ContextNode::~ContextNode() {
    delete nullNode;
}
*/


// setters 
void ContextNode::setChildren(ContextNode* child) {
    if(child != nullptr) {
        children.insert(children.end(), child);
    }

}

void ContextNode::setParent(ContextNode* parent) {
    this->parent = parent;
}

void ContextNode::setCallCount() {
    callCount++;
}

// getters

std::string ContextNode::functionName() {
    return funcName;
};

std::vector<std::string> ContextNode::getParameters() {
    return parameters;
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

ContextNode* ContextNode::getParent() {
    if(parent == nullptr) {
        return nullNode;
    } 
    return parent;
}

std::vector<ContextNode*> ContextNode::getChildren() {
    if (children.size() == 0) {
        children.insert(children.end(), nullNode);
    }
    return children;
}

int ContextNode::getCallCount(){
    return callCount;
}



// This class represents each node that will be part of the tree
// Another class should be built that takes a vector of the ContextNode and forms a tree.

