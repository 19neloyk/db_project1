#ifndef TABLE_H
#define TABLE_H
#include "recordtype.h"
#include "block.h"
using namespace std;

struct Table{
    RecordType* dataType;
    Block** recordBlocks;

    // I NEED A DIFFERENT CLASS TYPE
    RecordType* indexType;
    Block** index;
    Table* createTable();

};

#endif