#ifndef BPT_H
#define BPT_H

#include <iostream>
#include <vector>
#include <bits/stdc++.h>
#include "pcm.h"
using namespace std;

#define kEY_MAX         ULONG_LONG_MAX
#define kEY_MIN         0
#define DUP             -1

struct VirNode                                                  // 4K-byte node
{
        uint64_t keyNums;                                       // number of keys in a node
        uint64_t numSlot;                                       // belongs to which memory slot 
        VirNode *parentBlock;       
        uint64_t *key;
        VirNode **childBlock;
};

class Bpt
{
public:
        Bpt(int porder, uint64_t memory_size); 
        void updateParentNode(VirNode *leftBlock, VirNode *rightBlock, uint64_t value);
        void splitBlock(VirNode *curBlock);
        int insertKey(VirNode *curBlock, uint64_t val);
        void redistributeBlock(VirNode *leftBlock, VirNode *rightBlock, bool isLeaf, int posOfLeftBlock, int whichIsCurBlock);
        void mergeBlock(VirNode *leftBlock, VirNode *rightBlock, bool isLeaf, int posOfRightBlock);
        void deleteNode(VirNode *curBlock, bool isLeaf, int curBlockPosition, bool &isRecycle);
        void deleteKey(VirNode *curBlock, uint64_t value, int curBlockPosition);
        void print(vector < VirNode* > Blocks);
        void printLeaf(VirNode *curBlock);
        void printTreeInformation(VirNode *curBlock);
        void find(VirNode *curBlock, uint64_t value);
        int binarySearch(VirNode *curBlock, uint64_t value, int dup);
        VirNode *getPreLeaf(VirNode *curBlock);
        void setKeyNums(VirNode *curBlock, uint64_t value);
        uint64_t getKeyNums(VirNode *curBlock);
        void setNumSlot(VirNode *curBlock, uint64_t slot);
        uint64_t getNumSlot(VirNode *curBlock);
        void setKey(VirNode *curBlock, int pos, uint64_t value);
        uint64_t getKey(VirNode *curBlock, int pos); 
        void setPointer(VirNode *curBlock, int pos, VirNode *ptr);
        VirNode *getPointer(VirNode *curBlock, int pos);
        void setParent(VirNode *curBlock, VirNode *ptr);
        VirNode *getParent(VirNode *curBlock);
        void initNode(VirNode *curBlock);
        void freeNode(VirNode *curBlock, int error_checking);
        bool checkIsLeaf(VirNode *curBlock);
        bool checkIsNoKey(VirNode *curBlock);
        bool checkIsRoot(VirNode *curBlock);

        int getOrder() const
        {
            return this->order;
        }

        void setKeyFound(const bool &found)
        {
            this->keyFound = found;
        }

        bool getKeyFound() const
        {
            return this->keyFound;
        }

        void setRoot(VirNode *root)
        {
            this->rootBlock = root;
        }

        VirNode *getRoot() const
        {
            return this->rootBlock;
        }

        void setNodeNums(int num)
        {
            this->nodeNums = num;
        }

        int getNodeNums() const
        {
            return this->nodeNums;
        }

        Pcm getPCM()
        {
            return this->pcm;
        }

private:        
        int order;                                          // number of pointers [max number of keys = order - 1]     
        bool keyFound;
        VirNode *rootBlock;                                 // root node 
        int nodeNums;
        Pcm pcm;    
};

#endif
