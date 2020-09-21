#include <iostream>
#include <fstream>
#include <math.h>
#include <ctype.h>
#include "bpt.h"
#include "pcm.h"

#define PADDING     32                                                                                  // keyNums + numSlot + parent + childBlock[order]
#define DEL_RATIO   100000

void insertKeys(Bpt &, ifstream &, int &, int &, double, uint64_t);
void deleteKeys(Bpt &, int &, uint64_t);
void findKey(Bpt &, int &, uint64_t);
void printTree(Bpt &);
void pirntLeafs(Bpt &);
void clearTree(Bpt &, int &);
void printTreeInfo(Bpt &);
void openFile(ifstream &, string);
void closeFile(ifstream &);
uint64_t getKey(string &);
uint64_t getMemorySize(string);

int main(int argc, char *argv[])                                                                        // argv[1] = memory size, argv[2] = the path of input trace
{  
        int order = (SLOT_SIZE - PADDING) / (PER_kEY_BYTE * 2);                                         // (order * 2 * 8) = 4096 - paddings
        cout << "\nThe order of the tree is: " << order << endl;
        
        Bpt bTree(order, getMemorySize(argv[1]));
       
        string menu = "\n1.Insert keys\n2.Delete a key\n3.Find a key\n4.Print the tree\n5.Print leafs\n6.Clear the tree\n7.Output the tree and PCM Information\n";                                                                  
        cout << menu << endl << endl;

        ifstream file;
        int ch, total_valid_keys = 0;
        time_t start, end;
        
        while(cin >> ch && getchar() != EOF)                                                            // input memu
        {
            time(&start);
            if(ch == 1)                                                                                
            {
                int dup_keys = 0, insert_loop = 0;
                double del_ratio = 0.0;
                uint64_t shift_key = 0;

                cout << "\nInput the deletion ratio( [0.1 ~ 0.9], if 0, no deletes, if 1, clear all): ";
                cin >> del_ratio;

                cout << "\nInput the insertion loop count(default 1): ";
                cin >> insert_loop;
           
                while(insert_loop)
                {
                    openFile(file, argv[2]);
                    if(total_valid_keys == 0)
                    { 
                        VirNode *root = new VirNode;
                        bTree.initNode(root);                                                          // re-allocate the root
                        bTree.setRoot(root);
                    }

                    insertKeys(bTree, file, total_valid_keys, dup_keys, del_ratio, shift_key);
                    shift_key++;
                    insert_loop--;
                    closeFile(file);
                }
              
                cout << "\nTotal duplicate keys: " << dup_keys << endl;
                cout << "Total valid keys: " << total_valid_keys << endl;                
            }

            else if(ch == 2)
            {
                deleteKeys(bTree, total_valid_keys, 0);
                if(bTree.getKeyFound())
                    cout << "Delete the key successfully!!!" << endl << endl;
            }

            else if(ch == 3)
            {
                findKey(bTree, total_valid_keys, 0);
                if(bTree.getKeyFound())
                    cout << "Find the key successfully!!!" << endl << endl;
            }

            else if(ch == 4)
            {
                printTree(bTree);
            }

            else if(ch == 5)
            {
                pirntLeafs(bTree);
            }
        
            else if(ch == 6)
            {
                clearTree(bTree, total_valid_keys);
            }

            else if(ch == 7)
            {
                printTreeInfo(bTree);
            }

            else
            {
                cout << "Wrong input!!!" << endl << endl;
            }

            time(&end);
            cout << "The execution time is: " << (double)(end -start) << " secs" << endl << endl;
            cout << menu << endl << endl;
        }
             
        return 0;
}

void insertKeys(Bpt &bTree, ifstream &file, int &total_valid_keys, int &dup_keys, double del_ratio, uint64_t shift_key)
{       
        uint64_t *del_keys = NULL;
        int grab_key_ratio = 0, del_length = 0, del_counter = 0, total_keys = 0; 
        static int del_round = 0;
        string word;

        if(del_ratio > 0 && del_ratio < 1)
        { 
            grab_key_ratio = 10 - del_ratio * 10;
            del_length = DEL_RATIO * del_ratio;
            if(!shift_key)
                cout << "\nThe size of delete keys array is " << del_length << " and save key ratio is " << grab_key_ratio << endl << endl;

            del_keys = new uint64_t [del_length];
            memset(del_keys, 0, sizeof(uint64_t) * del_length);         
        }
        
        while(file >> word)
        {    
            total_keys++;
            uint64_t key = getKey(word);
            key += shift_key;                                                                               // to enlarge the size of input trace
            
            int ret;
            if(key)
                ret = bTree.insertKey(bTree.getRoot(), key);
        
            else
                continue;

            if(ret == DUP)
            {                              
                //cout << "Error: have the same key: " << key << endl << endl;
                dup_keys++;
            }

            else
            {   
                total_valid_keys++;
                if(grab_key_ratio && (total_keys % grab_key_ratio) == 0 && del_counter < del_length)
                {
                    del_keys[del_counter] = key;
                    del_counter++;                   
                }
            }

            if(grab_key_ratio && (total_keys % DEL_RATIO) == 0 && del_counter < total_valid_keys)
            {                 
                int deleteCounts = 0;
                for(int i = 0; i < del_counter; i++)
                {                  
                    deleteKeys(bTree, total_valid_keys, del_keys[i]);
                    deleteCounts++;  
                }

                del_counter = 0;
                memset(del_keys, 0, sizeof(uint64_t) * del_length);

                del_round++;
                cout << "The " << del_round << " deletion round and current valid key number " << total_valid_keys << endl;
                cout << "The deleted key number in this round " << deleteCounts << " and total dup keys " << dup_keys << endl << endl;          
            }      
        }         

        if(del_keys != NULL)
            delete[] del_keys;    

        if(del_ratio == 1)
            clearTree(bTree, total_valid_keys);
}

void deleteKeys(Bpt &bTree, int &total_valid_keys, uint64_t value)
{
        bTree.setKeyFound(false);
        if(total_valid_keys == 0)
        {           
            cout << "\nThere is no more key to be deleted!!!" << endl << endl;
            return;
        }

        else
        {
            if(!value)
            {
                cout << "\nDelete the key: ";
                cin >> value;
                cout << endl << endl;
            }

            /*bTree.find(bTree.getRoot(), value);
            if(!bTree.getKeyFound())
            {
                cout << "\nThe key not to be deleted is: " << value << endl << endl;
                return;
            }*/

            bTree.deleteKey(bTree.getRoot(), value, 0);
            total_valid_keys--; 

            if(total_valid_keys == 0)
            {
                cout << "\nDelete the root node successfully!!" << endl << endl;
                bTree.setRoot(NULL);          
            }    
        }        
}

void findKey(Bpt &bTree, int &total_valid_keys, uint64_t value)
{
        bTree.setKeyFound(false);
        if(total_valid_keys == 0)
        {           
            cout << "\nThere is no more key to be found!!!" << endl << endl;
            return;
        }

        if(!value)
        {
            cout << "\nInput the key: ";
            cin >> value;
            cout << endl << endl;
        }

        bTree.find(bTree.getRoot(), value);
        if(!bTree.getKeyFound())
            cout << "\nThe key not to be found is: " << value << endl << endl;
}

void printTree(Bpt &bTree)
{
        if(bTree.getRoot() != NULL)
        {
            vector < VirNode* > blocks;
            blocks.push_back(bTree.getRoot());
            bTree.print(blocks);
            cout << endl;
        }

        else
            cout << "\nThe tree is empty" << endl << endl;
}

void pirntLeafs(Bpt &bTree)
{
        if(bTree.getRoot() != NULL)
            bTree.printLeaf(bTree.getRoot());
        
        else
            cout << "\nThe tree is empty and no leafs" << endl << endl;
}

void clearTree(Bpt &bTree, int &total_valid_keys)
{
        if(total_valid_keys == 0)
        {           
            cout << "\nThere is no more key to be clear!!!" << endl << endl;
            return;
        }

        while(bTree.getRoot() != NULL)
        {
            bTree.setKeyFound(false);
            deleteKeys(bTree, total_valid_keys, bTree.getRoot()->key[0]);       
        }
}

void printTreeInfo(Bpt &bTree)
{
        if(bTree.getRoot() != NULL)
            bTree.printTreeInformation(bTree.getRoot());
        
        else
            cout << "\nThe tree is empty and no tree metadatas" << endl << endl;

        bTree.getPCM().printPCM();
}

void openFile(ifstream &file, string file_name)
{       
        file.open(file_name.c_str());
        if(!file.is_open())
        {
            cout << "\nError: fail to open file!!!" << endl << endl;
        }           
}

void closeFile(ifstream &file)
{
        file.close();
}

uint64_t getKey(string &word)
{
        if(word.empty())
        { 
            cout << "\nError: have a null key!!!" << endl << endl;
            return -1;
        }

        word.erase(0, 2);
        word.erase(word.size() - 2);

        for(int i = 0; i < word.length(); i++) 
            if (isdigit(word[i]) == false) 
            {    
                cout << "\nError: the key is not digits!!!" << endl << endl;
                return -1;
            }

        uint64_t key = atoll(word.c_str());                   
        if(key >= kEY_MAX || key <= kEY_MIN)
        {    
            cout << "\nThe key is out of range!!!" << endl << endl;
            return -1;
        }

        return key;   
}

uint64_t getMemorySize(string mSize)
{
        char mOrder = toupper(mSize[mSize.size() - 1]);
        mSize.erase(mSize.size() - 1);
        
        uint64_t memory_size = atoll(mSize.c_str());
        if(mOrder == 'K')
            memory_size = memory_size * 1024;

        else if(mOrder == 'M')
            memory_size = memory_size * 1024 * 1024;

        else if(mOrder == 'G')
            memory_size = memory_size * 1024 * 1024 * 1024;

        else
        {
            cout << "\nThe input memory size is wrong!!!" << endl << endl;
            return -1;
        }

        //cout << "The order and memory size is " << mOrder << "  " << memory_size << endl;
        return memory_size;
}
