#ifndef CONTEXTTREE_H
#define CONTEXTTREE_H

#include <string>
#include "ContextNode.h" 

class ContextTree {

private: 
    ContextNode root;


public:
    ContextTree(ContextNode node);

    void buildTree();

};

#endif // CONTEXTTREE_H