#ifndef PCM_H
#define PCM_H

#include <iostream>
#include <fstream>
#include <inttypes.h>
#include <queue>
#include <string.h>
using namespace std;

#define PER_kEY_BYTE    8
#define SLOT_SIZE       4096

struct MemSlot                                                      // 4k-byte slot
{
        uint64_t keyNums_counts;                                    // the numbers of keys
        uint64_t slot_counts;                                       // the allocation numbers of the MemSlot
        uint64_t parent_padding;                                           
        uint64_t *access_count;                                     // every 8 btyes
};

class Pcm
{
public:
        Pcm(int porder, uint64_t memorySize);
        uint64_t allocateSlot();
        void freeSlot(uint64_t slot_pos);
        void accessKey(uint64_t slot_pos, int access_pos);
        void accessPointer(uint64_t slot_pos, int access_pos); 
        void accessNodeCounter(uint64_t slot_pos);    
        uint64_t getSlotNodeCounter(uint64_t slot_pos);
        uint64_t getSlotAlloCount(uint64_t slot_pos);
        uint64_t getSlotWriteCount(uint64_t slot_pos, int access_pos);
        void printPCM();
        bool memSlotCheckError(uint64_t slot_pos);

        uint64_t getMaxSlot() const
        {
            return this->maxSlot;
        }

        uint64_t getMaxAccessCount() const
        {
            return this->maxAccessCount;
        }

        void addReadCounts()
        {
            this->totalPcmReadCounts++;
        }

        uint64_t getTotalReadCounts() const
        {
            return this->totalPcmReadCounts;
        }

        void addSlotAllocations()
        {
            this->totalPcmAllocations++;
        }

        uint64_t getTotalAllocations() const
        {
            return this->totalPcmAllocations;
        }

        void addSlotWriteCounts()
        {
            this->totalPcmWriteCounts++;
        }

        uint64_t getTotalWriteCounts() const
        {
            return this->totalPcmWriteCounts;
        }
     
private:
        int order;
        uint64_t maxSlot;
        MemSlot *memSlot;
        queue<uint64_t> qSlot;
        uint64_t maxAccessCount;
        uint64_t totalPcmReadCounts;
        uint64_t totalPcmAllocations;
        uint64_t totalPcmWriteCounts;
};

#endif
