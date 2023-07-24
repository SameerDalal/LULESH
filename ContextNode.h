#ifndef CONTEXTNODE_H
#define CONTEXTNODE_H

#include <string>
#include <vector>
#include <libunwind.h>

class ContextNode {
private:
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
    int callCount = 0;
public:
    ContextNode(std::string funcName = "",
                std::vector<std::string> parameters = {},
                unw_word_t start_ip = {},
                unw_word_t end_ip = {},
                unw_word_t lsda = {},
                unw_word_t handler = {},
                unw_word_t global_pointer = {},
                unw_word_t flags = {});

    void setChildren(ContextNode* child = nullptr);
    void setParent(ContextNode* parent = nullptr);
    void setCallCount();

    std::string functionName();
    std::vector<std::string> getParameters();
    unw_word_t getStartIP() const;
    unw_word_t getEndIP() const;
    unw_word_t getLsda() const;
    unw_word_t getHandler() const;
    unw_word_t getGlobalPointer() const;
    unw_word_t getFlags() const;

    ContextNode* getParent();
    std::vector<ContextNode*> getChildren();
    int getCallCount();
};

#endif // CONTEXTNODE_H
