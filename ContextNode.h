#ifndef CONTEXTNODE_H
#define CONTEXTNODE_H

#include <string>
#include <vector>
#include <libunwind.h>

class ContextNode {
    
private:

    ContextNode* parent;
    std::vector<ContextNode*> children;
    void* return_addr; //this is used to uniquely identify a call location
    long callCount;

    void* frame_addr;
    std::string funcName;
    unw_word_t start_ip;
    unw_word_t end_ip;
    unw_word_t lsda;
    unw_word_t handler; 
    unw_word_t global_pointer;
    unw_word_t flags;
    std::vector<std::string> parameters;
    std::vector<std::string> arguments;

    bool parallelBlock;

    std::vector<std::string> color;

public:
    ContextNode(        void* frame_addr, 
                        void* return_addr, 
                        std::string funcName,
                        unw_word_t start_ip,
                        unw_word_t end_ip,
                        unw_word_t lsda,
                        unw_word_t handler,
                        unw_word_t global_pointer,
                        unw_word_t flags,
                        std::vector<std::string> parameters);


    ContextNode(std::string funcName, void* return_addr);

    ~ContextNode();

    void addChild(ContextNode* node);
    void removeChild();

    void setParentNode(ContextNode* node);

    std::string getFunctionName();
    ContextNode* getParentNode();

    std::vector<std::string> & getParameters();

    std::vector<std::string> &getArguments();
    void addArgument(std::string arg);

    std::vector<ContextNode*> getChildren();

    void setArguments(std::vector<std::string> args);

    void* getFrameAddress();
    void* getReturnAddress();

    void addCallCount(long val);

    long getCallCount();

    unw_word_t getStartIP() const;
    unw_word_t getEndIP() const;
    unw_word_t getLsda() const;
    unw_word_t getHandler() const;
    unw_word_t getGlobalPointer() const;
    unw_word_t getFlags() const;

    void setParallelBlock(bool isParallel);
    
    bool getParallelBlock();

    std::vector<std::string> getColor();

    void setColor(std::string color);
};

#endif