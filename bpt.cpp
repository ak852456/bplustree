#include "bpt.h"

Bpt::Bpt(int porder, uint64_t memory_size)                                                                      // constructor 
    :order(porder), keyFound(false), rootBlock(NULL), nodeNums(0), pcm(porder, memory_size) 
{  

}

void Bpt::updateParentNode(VirNode *leftBlock, VirNode *rightBlock, uint64_t value)                            
{ 
        if(checkIsRoot(leftBlock))                                                                              // if the curBlock is rootBlock now
        {           
            VirNode *parentBlock = new VirNode;
            initNode(parentBlock);

            setKeyNums(parentBlock, 1);
            setKey(parentBlock, 0, value);
            setPointer(parentBlock, 0, leftBlock);
            setPointer(parentBlock, 1, rightBlock);

            setParent(leftBlock, parentBlock);
            setParent(rightBlock, parentBlock);          
            setRoot(parentBlock);             
        }

        else                                                                                                    // assign the leftBlock and the rightBlock to the exact place in the parentBlock 
        {            
            VirNode *parentBlock = getParent(leftBlock);                                                           
            int pos = binarySearch(parentBlock, value, 0);
            int nums = getKeyNums(parentBlock);

            for(int i = nums - 1; i >= pos; i--)
            {    
                uint64_t key = getKey(parentBlock, i);
                setKey(parentBlock, i + 1, key);
            }

            setKey(parentBlock, pos, value);           
            for(int j = nums; j >= pos + 1; j--)                                                               
            {    
                VirNode *childBlock = getPointer(parentBlock, j);
                setPointer(parentBlock, j + 1, childBlock);
            }

            setPointer(parentBlock, pos + 1, rightBlock);
            setPointer(parentBlock, pos , leftBlock);
            setKeyNums(parentBlock, nums + 1);  
        }                                     
}

void Bpt::splitBlock(VirNode *curBlock)
{
        int i, j, nums_in_left = getOrder() / 2;                                                                // split the less half to the leftBlock when an order is odd(equal when an order is even)   
        uint64_t parent_key, key; 
        bool isLeaf = checkIsLeaf(curBlock);
        VirNode *parentBlock = getParent(curBlock), *childBlock = NULL;
                           
        VirNode *leftBlock = new VirNode;                                                                      
        initNode(leftBlock);

        VirNode *rightBlock = new VirNode;                                                                      
        initNode(rightBlock);

        setKeyNums(leftBlock, nums_in_left);
        setKeyNums(rightBlock, getOrder() - nums_in_left);
        setParent(leftBlock, parentBlock);
        setParent(rightBlock, parentBlock);

        for(i = 0; i < nums_in_left; i++)
        {    
            key = getKey(curBlock, i);
            setKey(leftBlock, i, key);
        }
    
        for(i = nums_in_left, j = 0; i < getOrder(); i++, j++)
        {    
            key = getKey(curBlock, i);
            if(i == nums_in_left)                                                                               // if non-leaf, get rid of the first key
            {
                parent_key = key;
                if(!isLeaf)
                {
                    setKeyNums(rightBlock, getKeyNums(rightBlock) - 1);
                    j--;
                    continue;
                }
            }   

            setKey(rightBlock, j, key);
        }

        if(isLeaf)                                                                                              // reset the leaf link
        {
            VirNode *nextLeaf = getPointer(curBlock, getOrder());
            if(nextLeaf != NULL)
                setPointer(rightBlock, getOrder(), nextLeaf);
            
            setPointer(leftBlock, getOrder(), rightBlock);

            VirNode *prevLeaf = getPreLeaf(curBlock);
            if(prevLeaf != NULL)
                setPointer(prevLeaf, getOrder(), leftBlock); 
        }

        else                                                                                                   
        {
            for(i = 0; i <= nums_in_left; i++)
            {
                childBlock = getPointer(curBlock, i);
                setPointer(leftBlock, i, childBlock);  
                setParent(childBlock, leftBlock); 
            }

            for(i = nums_in_left + 1, j = 0; i <= getOrder(); i++, j++)
            {
                childBlock = getPointer(curBlock, i);
                setPointer(rightBlock, j, childBlock);  
                setParent(childBlock, rightBlock); 
            }
        }
  
        updateParentNode(leftBlock, rightBlock, parent_key); 
        freeNode(curBlock, -1);
}

int Bpt::insertKey(VirNode *curBlock, uint64_t value)
{       
        int pos = binarySearch(curBlock, value, 1);
        if(pos == DUP)                                                                                          // find the duplicate key
        {
            return DUP;
        }

        VirNode *childBlock = getPointer(curBlock, pos);
        if(childBlock != NULL)                                                                                  // index node
        {   
            pos = insertKey(childBlock, value);     
            if(pos == DUP)
            {
                return DUP;
            }  
        }

        else                                                                                                    // leaf node
        {   
            uint64_t nums = getKeyNums(curBlock);
            for(int i = nums - 1; i >= pos; i--)
            {    
                uint64_t key = getKey(curBlock, i);
                setKey(curBlock, i + 1, key);
            }  
            
            setKey(curBlock, pos, value);
            setKeyNums(curBlock, nums + 1);
        }    
      
        if(getKeyNums(curBlock) == getOrder())
        {   
            splitBlock(curBlock);            
        }
  
        return 0;
}

void Bpt::redistributeBlock(VirNode *leftBlock, VirNode *rightBlock, bool isLeaf, int posOfLeftBlock, int whichIsCurBlock)
{
        VirNode *childBlock = NULL, *parentBlock = getParent(leftBlock); 
        uint64_t val, leftNums = getKeyNums(leftBlock), rightNums = getKeyNums(rightBlock);

        if(whichIsCurBlock == 0)                                                                                // LeftBlock is curBlock, borrow the key from rightBlock
        { 
            if(!isLeaf)
            {
                val = getKey(parentBlock, posOfLeftBlock);                                                      // take down the key from parentBlock                                   
                setKey(leftBlock, leftNums, val);

                childBlock = getPointer(rightBlock, 0);
                setPointer(leftBlock, leftNums + 1, childBlock);
                setParent(childBlock, leftBlock);
                
                val = getKey(rightBlock, 0);                                                                    // send up the first key in rightBlock to the parentBlock                                  
                setKey(parentBlock, posOfLeftBlock, val);

                for(int i = 0; i < rightNums; i++)                                                                                                                                                               
                {
                    val = getKey(rightBlock, i + 1);                                                                                            
                    setKey(rightBlock, i, val);
    
                    childBlock = getPointer(rightBlock, i + 1);
                    setPointer(rightBlock, i, childBlock);      
                }

                childBlock = getPointer(rightBlock, rightNums + 1);
                setPointer(rightBlock, rightNums, childBlock);
            }

            else
            { 
                val = getKey(rightBlock, 0);                                                                    // borrow the first key in rightBlock to the last position in leftBlock                                   
                setKey(leftBlock, leftNums, val);
                
                for(int i = 0; i < rightNums; i++)                                                              // left shift by one in rightBlock 
                {
                    val = getKey(rightBlock, i + 1);                                                                                                          
                    setKey(rightBlock, i, val);
                }
                    
                val = getKey(rightBlock, 0);                                                                                                      
                setKey(parentBlock, posOfLeftBlock, val);
            }

            setKeyNums(leftBlock, leftNums + 1);
            setKeyNums(rightBlock, rightNums - 1);
        }
            
        else                                                                                                    // RightBlock is curBlock, borrow the key from leftBlock
        { 
            if(!isLeaf)
            {
                childBlock = getPointer(rightBlock, rightNums);
                setPointer(rightBlock, rightNums + 1, childBlock);

                for(int i = rightNums - 1; i >= 0; i--)                                                                                                           
                {
                    val = getKey(rightBlock, i);                                                                                                      
                    setKey(rightBlock, i + 1, val);

                    childBlock = getPointer(rightBlock, i);
                    setPointer(rightBlock, i + 1, childBlock); 
                }
 
                val = getKey(parentBlock, posOfLeftBlock);                                                      // bring down the key from parentBlock to first position in rightBlock
                setKey(rightBlock, 0, val);

                childBlock = getPointer(leftBlock, leftNums);                                                   // the new first childBlock in rightBlock will be the last childBlock of leftBlock
                setPointer(rightBlock, 0, childBlock);
                setParent(childBlock, rightBlock);
                setPointer(leftBlock, leftNums, NULL);

                val = getKey(leftBlock, leftNums - 1);                                                          // send up the last key in leftBlock to the parentBlock
                setKey(parentBlock, posOfLeftBlock, val);            
            }

            else
            {
                for(int i = rightNums - 1; i >= 0; i--)                                                         // right shift by one in rightBlock so that first position becomes free
                {
                    val = getKey(rightBlock, i);                                                                                                      
                    setKey(rightBlock, i + 1, val);
                }
               
                val = getKey(leftBlock, leftNums - 1);                                                          // borrow the last key in leftBlock to the first position in rightBlock
                setKey(rightBlock, 0, val);
                setKey(parentBlock, posOfLeftBlock, val);                                                       // substitute the key in parentBlock
            }

            setKeyNums(rightBlock, rightNums + 1);
            setKey(leftBlock, leftNums - 1, kEY_MAX);
            setKeyNums(leftBlock, leftNums - 1); 
        }     
}

void Bpt::mergeBlock(VirNode *leftBlock, VirNode *rightBlock, bool isLeaf, int posOfRightBlock)
{         
        VirNode *childBlock = NULL, *parentBlock = getParent(leftBlock);
        int i, j;
        uint64_t key, leftNums = getKeyNums(leftBlock), rightNums = getKeyNums(rightBlock), parentNums;
        
        uint64_t newSlot = pcm.allocateSlot();                                                                  // allocate a new memory slot
        pcm.freeSlot(leftBlock->numSlot);
        setNumSlot(leftBlock, newSlot);
        
        for(i = 0; i < leftNums; i++)
        {    
            key = getKey(leftBlock, i);                                    
            setKey(leftBlock, i, key);
        }

        for(int i = 0; i <= leftNums; i++)
        {     
            childBlock = getPointer(leftBlock, i);
            setPointer(leftBlock, i, childBlock);
        }

        if(!isLeaf)
        {
            key = getKey(parentBlock, posOfRightBlock - 1);                                                     // take down the key in parentBlock
            setKey(leftBlock, leftNums, key);
            leftNums++;
            setKeyNums(leftBlock, leftNums);
        }

        for(i = leftNums, j = 0; j < rightNums; i++, j++)
        {    
            key = getKey(rightBlock, j);                                    
            setKey(leftBlock, i, key);
        }

        if(!isLeaf)
        {
            for(i = leftNums, j = 0; j <= rightNums; i++, j++)
            { 
                childBlock = getPointer(rightBlock, j);
                setPointer(leftBlock, i, childBlock); 
                setParent(childBlock, leftBlock);
            }
        }
    
        setKeyNums(leftBlock, leftNums + rightNums);
        if(isLeaf)                                                                                              // reset leaf link
        {
            VirNode *nextLeaf = getPointer(rightBlock, getOrder());
            setPointer(leftBlock, getOrder(), nextLeaf); 
        }

        parentNums = getKeyNums(parentBlock);
        for(i = posOfRightBlock - 1; i < parentNums; i++)                                                       // left shift the key and the pointer by one in parentBlock                   
        {
            key = getKey(parentBlock, i + 1);                                    
            setKey(parentBlock, i, key);                
        }

        for(i = posOfRightBlock; i <= parentNums; i++)                                                                
        {
            childBlock = getPointer(parentBlock, i + 1);
            setPointer(parentBlock, i, childBlock); 
        }
           
        setKeyNums(parentBlock, parentNums - 1); 
}

void Bpt::deleteNode(VirNode *curBlock, bool isLeaf, int curBlockPosition, bool &isRecycle)
{       
        VirNode *leftBlock = NULL, *rightBlock = NULL, *parentBlock = getParent(curBlock);              
        
        if(curBlockPosition != 0)                                                                               // find its neighbor leftBlock                            
        {    
            leftBlock = getPointer(parentBlock, curBlockPosition - 1);                                                                
            pcm.addReadCounts();
        }

        if(curBlockPosition != getKeyNums(parentBlock))                                                         // find its neighbor rightBlock
        {    
            rightBlock = getPointer(parentBlock, curBlockPosition + 1); 
            pcm.addReadCounts();
        }

        if(leftBlock != NULL && getKeyNums(leftBlock) >= ceil((double)getOrder() / 2))                          // redistribute first
        {
            redistributeBlock(leftBlock, curBlock, isLeaf, curBlockPosition - 1, 1);
        }
    
        else if(rightBlock != NULL && getKeyNums(rightBlock) >= ceil((double)getOrder() / 2))
        { 
            redistributeBlock(curBlock, rightBlock, isLeaf, curBlockPosition, 0);
        }

        else if(leftBlock != NULL)                                                                              // if it can't redistribute, try to merge 
        {
            mergeBlock(leftBlock, curBlock, isLeaf, curBlockPosition);
            isRecycle = true;
        }

        else if(rightBlock != NULL)
        {  
            mergeBlock(curBlock, rightBlock, isLeaf, curBlockPosition + 1);
            freeNode(rightBlock, -5);
        }     
}

void Bpt::deleteKey(VirNode *curBlock, uint64_t value, int curBlockPosition)                                    // curBlockPosition is the position of curBlock in its parentBlock
{       
        bool isLeaf = false;
        if(checkIsLeaf(curBlock))                                                                            
            isLeaf = true;

        // the leftmost key could be changed due to merge or redistribute later,
        // so keep it to check and replace it's copy from it's ancestor

        bool isLeftMost = false;
        uint64_t prevLeftMostVal = getKey(curBlock, 0);
        if(prevLeftMostVal == value && isLeaf)
            isLeftMost = true;

        int pos = binarySearch(curBlock, value, 0);
        VirNode *childBlock = getPointer(curBlock, pos);
        
        if(childBlock != NULL)                                                                    
        {   
            deleteKey(childBlock, value, pos);     
        }

        else                                                                                                      
        {   
            setKeyFound(true);
            int nums = getKeyNums(curBlock);
            for(int i = pos; i < nums; i++)                                                                     // delete the key by left shifting all keys and pointers by one
            {
                uint64_t key = getKey(curBlock, i + 1);
                setKey(curBlock, i, key);
            }

            setKeyNums(curBlock, nums - 1);
            if(checkIsNoKey(curBlock))
            {           
                freeNode(curBlock, -2);   
                return;
            }
        }

        bool isRoot = false;
        if(checkIsRoot(curBlock))                                                                            
            isRoot = true;
        
        if(isRoot && isLeaf)                                                                                    // the only node is root
        {  
            return;          
        }

        if(isRoot && !isLeaf && checkIsNoKey(curBlock))                                                         // after deleting, the leftBlock of root becomes as root                  
        {     
            VirNode *leftMostChild = getPointer(curBlock, 0);
            setRoot(leftMostChild);
            setParent(rootBlock, NULL);

            freeNode(curBlock, -3);
            return;
        }
         
        // check if the curBlock has less than half of the number of maximum keys
        bool recycle = false;
        if(getKeyNums(curBlock) < ceil((double)getOrder() / 2) - 1 && !isRoot)                                  // root can has least one key
        {
            deleteNode(curBlock, isLeaf, curBlockPosition, recycle);     
        }
           

//      1.  (7, 10)         after deleting 7    2.  (7, 10)         must be    3.  (8, 10)          
//             |                                       |                              |
//             |                                       |                              |
//          (7, 8, 9)                               (8, 9)                         (8, 9)

// if the deleted key is leftmost in leaf, delete the duplicate if any in the ancestor block

        if(isLeftMost && !recycle)
        {
            VirNode *dupBlock = getParent(curBlock);                                                                
            while(dupBlock != NULL)
            {
                pcm.addReadCounts();
                int nums = getKeyNums(dupBlock);

                for(int i = 0; i < nums; i++)
                {
                    if(getKey(dupBlock, i) == prevLeftMostVal)
                    {
                        uint64_t key = getKey(curBlock, 0);
                        setKey(dupBlock, i, key);
                        break;
                    }
                }

                dupBlock = getParent(dupBlock);
            }
        }

        if(recycle)
        {
            freeNode(curBlock, -4);
        }
}

void Bpt::find(VirNode *curBlock, uint64_t value)
{
        int pos = binarySearch(curBlock, value, 0);
        VirNode *child = getPointer(curBlock, pos);

        if(child != NULL)                                                                    
        {   
            find(child, value);     
        }

        else                                                                                                      
        {   
            if(value == getKey(curBlock, pos))
                setKeyFound(true);
        }        
}

void Bpt::print(vector < VirNode* > blocks)
{       
        vector < VirNode* > newBlocks;
        for(int i = 0; i < blocks.size(); i++)                                                                  // for every block
        { 
            VirNode *curBlock = blocks[i]; 
            cout << "[|";

            int nums = getKeyNums(curBlock);
            for(int j = 0; j <= nums; j++)                                                                      // traverse the childBlocks, print keys and save all the childBlocks
            {  
                uint64_t key = getKey(curBlock, j);
                if(key != kEY_MAX)
                    cout << key << "|";
    
                VirNode *child = getPointer(curBlock, j);
                if(child != NULL)
                {   
                    newBlocks.push_back(child);
                }
            }
                    
            cout << "]  ";
        }

        cout << endl << endl;
        blocks.clear();

        if(newBlocks.size() != 0)                                                                               // put the childBlocks to the recursion to continue to the more depth
            print(newBlocks);
}

void Bpt::printLeaf(VirNode *curBlock)
{
        while(!checkIsLeaf(curBlock))
            curBlock = getPointer(curBlock, 0);

        do
        {
            cout << "|";

            int nums = getKeyNums(curBlock);
            for(int i = 0; i < nums; i++)
               cout << " " << getKey(curBlock, i) << " ";

            cout << "|";
            curBlock = getPointer(curBlock, getOrder());
        }
        while(curBlock != NULL);

        cout << endl << endl;
}

void Bpt::printTreeInformation(VirNode *curBlock)
{
        int height = 0, totalKeys = 0;
        uint64_t min = kEY_MAX, max = kEY_MIN;

        while(!checkIsLeaf(curBlock))
        {    
            curBlock = getPointer(curBlock, 0);
            height++;
        }

        do
        {
            int nums = getKeyNums(curBlock);
            for(int i = 0; i < nums; i++)                                                             
            {  
                uint64_t key = getKey(curBlock, i); 
                if(key < min)
                    min = key;

                if(key > max)
                    max = key;

                totalKeys++;
            }

            curBlock = getPointer(curBlock, getOrder());
        }
        while(curBlock != NULL);

        cout << "\nThe height of the tree is: " << height << endl;
        cout << "The total of keys in leafs is: " << totalKeys << endl;
        cout << "The total of nodes in the tree is: " << getNodeNums() << endl;
        cout << "The Max key in the tree is: " << max << endl;
        cout << "The Min key in the tree is: " << min << endl << endl;
}

int Bpt::binarySearch(VirNode *curBlock, uint64_t value, int dup)                                               // find the exact insertion position
{
        pcm.addReadCounts();
        uint64_t key;
        int mid = 0, low = 0, high = getKeyNums(curBlock);

        while(low <= high)
        { 
            mid = (low + high) / 2;  
            key = getKey(curBlock, mid);          
            
            if(key > value)
            {
                high = mid - 1;
            }

            else if(key < value)
            {

                low = mid + 1;
            }

            else
            {
                if(dup)                                                                                         // check duplicate keys
                    return DUP;

                else
                {    
                    if(checkIsLeaf(curBlock))
                    {
                        return mid;
                    }

                    else                                                                                        // find the key in a non-leaf block
                    {               
                        return mid + 1;
                    }
                }
            }
        }

        return key > value ? mid : mid + 1;                                                                     // find the closest position
}

VirNode *Bpt::getPreLeaf(VirNode *curBlock)
{
        VirNode *prev = NULL;
        if(getRoot())
            prev = getPointer(getRoot(), 0);

        if(prev != NULL)
            while(getPointer(prev, 0) != NULL)
                prev = getPointer(prev, 0);

        while(prev != NULL)
        {       
            if(getPointer(prev, getOrder()) == curBlock)
            { 
                break;
            }

            else
                prev = getPointer(prev, getOrder());
        }        

        return prev;
}

void Bpt::setKeyNums(VirNode *curBlock, uint64_t value)
{
        pcm.accessNodeCounter(getNumSlot(curBlock));
        curBlock->keyNums = value;
}

uint64_t Bpt::getKeyNums(VirNode *curBlock)
{
        return curBlock->keyNums;
}

void Bpt::setNumSlot(VirNode *curBlock, uint64_t slot)
{
        curBlock->numSlot = slot;
}

uint64_t Bpt::getNumSlot(VirNode *curBlock)
{
        return curBlock->numSlot;
}
        
void Bpt::setKey(VirNode *curBlock, int pos, uint64_t value)                                                                   
{
        if(pos < 0 || pos >= getOrder())
            cout << "Error: the wrong position to set the key " << pos << endl << endl;

        pcm.accessKey(getNumSlot(curBlock), pos * 2 + 1);
        curBlock->key[pos] = value;
}
        
uint64_t Bpt::getKey(VirNode *curBlock, int pos)                                                                               
{
        if(pos < 0 || pos >= getOrder())
            cout << "Error: the wrong position to get the key " << pos << endl << endl;

        return curBlock->key[pos];
}

void Bpt::setPointer(VirNode *curBlock, int pos, VirNode *ptr)
{
        if(pos < 0 || pos > getOrder())
            cout << "Error: the wrong position to set the pointer " << pos << endl << endl;

        pcm.accessPointer(getNumSlot(curBlock), pos * 2);
        curBlock->childBlock[pos] = ptr;
}
        
VirNode * Bpt::getPointer(VirNode *curBlock, int pos)
{
        if(pos < 0 || pos > getOrder())
            cout << "Error: the wrong position to get the pointer " << pos << endl << endl;

        return curBlock->childBlock[pos];
}

void Bpt::setParent(VirNode *curBlock, VirNode *ptr)
{
        curBlock->parentBlock = ptr;
}

VirNode * Bpt::getParent(VirNode *curBlock)
{
        return curBlock->parentBlock;
}

void Bpt::initNode(VirNode *curBlock)
{
        curBlock->keyNums = 0;
        curBlock->parentBlock = NULL;

        curBlock->key = new uint64_t [getOrder()];
        if(!curBlock->key)
            cout << "\nError: fail to allocate keys" << endl << endl;

        curBlock->childBlock = new VirNode *[getOrder() + 1];
        if(!curBlock->childBlock)
            cout << "\nError: fail to allocate childBlocks" << endl << endl;

        curBlock->numSlot = pcm.allocateSlot();
        for(int i = 0; i < getOrder(); i++)    
        {
            curBlock->key[i] = kEY_MAX;                                                                         // 2 ^ 64 - 1
            curBlock->childBlock[i] = NULL;
        }

        curBlock->childBlock[getOrder()] = NULL;     
        setNodeNums(getNodeNums() + 1);     
}

void Bpt::freeNode(VirNode *curBlock, int error_checking)
{
        pcm.freeSlot(curBlock->numSlot);
        curBlock->numSlot = curBlock->keyNums = 0;
        curBlock->parentBlock = NULL;      
        
        delete[] curBlock->key;
        for(int i = 0; i < getOrder() + 1; i++)    
        {
            curBlock->childBlock[i] = NULL;
            delete curBlock->childBlock[i];
        }   
                    
        delete[] curBlock->childBlock;
        delete curBlock;
        curBlock = NULL;
        setNodeNums(getNodeNums() - 1); 
}

bool Bpt::checkIsLeaf(VirNode *curBlock)
{
        if(getPointer(curBlock, 0) == NULL)
        {              
            return true;
        }

        return false;
}

bool Bpt::checkIsNoKey(VirNode *curBlock)
{
        if(getKeyNums(curBlock) == 0)
        {   
            return true;
        }

        return false;
}

bool Bpt::checkIsRoot(VirNode *curBlock)
{
        if(getParent(curBlock) == NULL)
        {   
            return true;
        }

        return false;
}
