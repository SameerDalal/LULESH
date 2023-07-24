#include <vector>
#include "ContextTree.h"

#include <iostream>


ContextTree::ContextTree(ContextNode node) : root(node) {
}



void ContextTree::buildTree() {
    std::cout << "context tree works" << std::endl;
}
 




// root, children, parent, sibling, leaf.