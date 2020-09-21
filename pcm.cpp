#include "pcm.h"

Pcm::Pcm(int porder, uint64_t memorySize)
{
        order = porder;
        maxSlot = memorySize / SLOT_SIZE;
        cout << "The maximum memory slot size is: " << maxSlot << endl;

        memSlot = new MemSlot [maxSlot];
        if(!memSlot)
            cout << "\nError: fail to allocate memory slots" << endl << endl;

        maxAccessCount = order * 2 + 1;
        totalPcmReadCounts = totalPcmAllocations = totalPcmWriteCounts = 0;

        for(int i = 0; i < maxSlot; i++)
        {
            qSlot.push(i);
            memSlot[i].keyNums_counts = memSlot[i].slot_counts = memSlot[i].parent_padding = 0;
            memSlot[i].access_count = new uint64_t [maxAccessCount];

            if(!memSlot[i].access_count)
                cout << "\nError: fail to allocate memory access count" << endl << endl;

            for(int j = 0; j < maxAccessCount; j++)
                memSlot[i].access_count[j] = 0;
        }
}

uint64_t Pcm::allocateSlot()
{       
        if(qSlot.size() == 0)
        {
            cout << "\nError: memory space is not enough!!!" << endl << endl;
            return -1;
        }

        uint64_t slot_pos = qSlot.front();
        qSlot.pop(); 
        memSlot[slot_pos].slot_counts++;
        
        addSlotAllocations();
        return slot_pos;
}

void Pcm::freeSlot(uint64_t slot_pos)
{
        if(memSlotCheckError(slot_pos))
            cout << "\nError: the wrong position to free!!!" << endl << endl;

        qSlot.push(slot_pos);
}

void Pcm::accessKey(uint64_t slot_pos, int access_pos)
{
        if(memSlotCheckError(slot_pos))
            cout << "\nError: the wrong position to access key!!!" << access_pos << endl << endl;
            
        if(access_pos % 2 == 0)
            cout << "\nError: not the correct key position!!!" << access_pos << endl << endl;

        memSlot[slot_pos].access_count[access_pos]++;
        addSlotWriteCounts();
}

void Pcm::accessPointer(uint64_t slot_pos, int access_pos)      
{
        if(memSlotCheckError(slot_pos))
            cout << "\nError: the wrong to position access pointer!!!" << access_pos << endl << endl;
        
        if(access_pos % 2 != 0)
            cout << "\nError: not the correct pointer position!!!" << access_pos << endl << endl;

        memSlot[slot_pos].access_count[access_pos]++;
        addSlotWriteCounts();
}

void Pcm::accessNodeCounter(uint64_t slot_pos)                 
{
        if(memSlotCheckError(slot_pos))
            cout << "\nError: the wrong position to access nodeCount!!!" << endl << endl;

        memSlot[slot_pos].keyNums_counts++;
}

uint64_t Pcm::getSlotNodeCounter(uint64_t slot_pos)
{
        if(memSlotCheckError(slot_pos))
            cout << "\nError: the wrong position to get nodeCount!!!" << endl << endl;

        return memSlot[slot_pos].keyNums_counts;
}

uint64_t Pcm::getSlotAlloCount(uint64_t slot_pos)
{
        if(memSlotCheckError(slot_pos))
            cout << "\nError: the wrong position to get slotAllocation!!!" << endl << endl;

        return memSlot[slot_pos].slot_counts;
}

uint64_t Pcm::getSlotWriteCount(uint64_t slot_pos, int access_pos)
{
        if(memSlotCheckError(slot_pos))
            cout << "\nError: the wrong position to access write count!!!" << endl << endl;

        if(access_pos < 0 || access_pos >= getMaxAccessCount())
            cout << "\nError: not thee correct write position!!!" << access_pos << endl << endl;

        return memSlot[slot_pos].access_count[access_pos];
}

void Pcm::printPCM()
{
        ofstream keyWriteFile, slotWriteFile, allocFile;
        string fileName = "Normal-750M-0.6-50rounds-";

        string fileName1 = fileName + "keyWriteCounts.txt";
        keyWriteFile.open(fileName1.c_str());
        if(!keyWriteFile.is_open())
        {
            cout << "\nError: fail to open file" << endl << endl;
        }

        string fileName2 = fileName + "slotWriteCounts.txt";
        slotWriteFile.open(fileName2.c_str());
        if(!slotWriteFile.is_open())
        {
            cout << "\nError: fail to open file" << endl << endl;
        }

        string fileName3 = fileName + "allocationCounts.txt";
        allocFile.open(fileName3.c_str());
        if(!allocFile.is_open())
        {
            cout << "\nError: fail to open file" << endl << endl;
        }

        for(uint64_t i = 0; i < getMaxSlot(); i++)
        {
            uint64_t keyAccessCounts_per_slot = 0;
            for(int j = 1; j < getMaxAccessCount(); j += 2)
            { 
                keyAccessCounts_per_slot += getSlotWriteCount(i, j);
            }

            slotWriteFile << keyAccessCounts_per_slot << endl;
            allocFile << getSlotAlloCount(i) << endl;
        }

        for(int j = 1; j < getMaxAccessCount(); j += 2)
        {
            uint64_t keyAccessCounts_per_8byte = 0;
            for(uint64_t i = 0; i < getMaxSlot(); i++)
            {
                keyAccessCounts_per_8byte += getSlotWriteCount(i, j);
            }

            keyWriteFile << keyAccessCounts_per_8byte << endl;
        }

        cout << "\nPCM the used bytes of memory is: " << getMaxSlot() * SLOT_SIZE << endl << endl;
        cout << "PCM total read counts: " << getTotalReadCounts() << endl << endl;

        cout << "PCM total memory slot allocations: " << getTotalAllocations() << endl << endl;  
        cout << "PCM the average slot allocation is " << getTotalAllocations() / getMaxSlot() << endl << endl;

        cout << "PCM the total memory write count: " << getTotalWriteCounts() << endl << endl;
        cout << "PCM the average slot write count is " << getTotalWriteCounts() / getMaxSlot() << endl << endl;

        keyWriteFile.close();
        slotWriteFile.close();
        allocFile.close();
}

bool Pcm::memSlotCheckError(uint64_t slot_pos)
{
        if(slot_pos < 0 || slot_pos >= getMaxSlot())
        {      
            cout << "\nError: the position that beyonds the memory range is " << slot_pos << endl << endl;
            return true;
        }

        return false;
}
