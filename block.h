#ifndef BLOCK_H
#define BLOCK_H
#include "recordtype.h"
using namespace std;

struct Block {
    int size;
    void* data;
    RecordType* dataType;
    bool isVariableLength;
    
    Block* createBlock();
    
    int addRecord;
    int recordCount;

    int updateRecord(Block* block);
    int findRecord(Block* block);
};

#endif