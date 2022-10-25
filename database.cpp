#include "database.h"
using namespace std;

Database* createDatabase(IndexType t, int blocksize) {
    Database* db = (Database*) malloc (sizeof(Database));
    db->type = t;
    db->numTables = 0;
    db->blockSize = blocksize;
    db->dbPrimaryBlock = (void*) malloc(db->blockSize);
    db->tableIndexMap = new map<string, int>();
    db->tableRecordTypeMap = new map<string, RecordType*>;
    return db;
}


bool addBlock(Database* db, void* tablePtr) {
    // First check if we can even add anymore blocks given
    // the size constraint of the tableRootPtr
    // (Calculated by looking at size in tableRootPtr after
    // the preliminary details, i.e. numEntries and numBlocks)
    int maxBlocks = (db->blockSize - sizeof(int) - sizeof(int))/sizeof(void*);
    int numBlocks = ((int*) tablePtr)[1];
    if (numBlocks == maxBlocks) {
        printf("We can only store %d block addresses with this block size, no more blocks allowed", maxBlocks);
        return false;
    }

    // Go to start of the block array
    // (after the two details in the beginning)
    void** blockArr = (void**) (((int*) tablePtr) + 2);
    blockArr[numBlocks] = (void*) malloc(db->blockSize);

    // Access the number of block variables and
    // increment it by 1
    ((int*) tablePtr)[1] ++;

    return;
}

template<typename... Args>void createTable(Database* db, const char *table_name, const char *primary_key, int length, Args&&... args) {
    // Check if there is already a table with the specified name
    string tableName = string(table_name);
    if (db->tableIndexMap->find(tableName) == db->tableIndexMap->end()) {
    // not found case
        printf("Table '%s' already exists, please use another name\n", tableName);
        return;
    } 

    // Create the specified record type and add into record type map
    RecordType* rt = createRecordType(primary_key, length, forward<Args>(args)...);
    db->tableRecordTypeMap->insert(make_pair(tableName, rt));

    // Now allocate memory for the table's root block and insert it
    // into the dbPrimaryBlock at the correct index
    void* tableRootBlockPtr = (void*) malloc (db->blockSize);
    db->dbPrimaryBlock[db->numTables] = tableRootBlockPtr;

    // Add block into the tableIndexMap
    db->tableIndexMap->insert(make_pair(tableName, db->numTables));
    
    // Increment number of tables
    db->numTables ++;

    // Now, we add the preliminary information to each table,
    // the number of entries and the number of blocks (will
    // be 1 right after allocating it)
    int numEntries = 0;
    int numBlocks = 0;

    ((int*) tableRootBlockPtr)[0] = numEntries;
    ((int*) tableRootBlockPtr)[1] = numBlocks;

    if (!addBlock(tableRootBlockPtr, tableRootBlockPtr)) {
        printf("Couldn't add block \n");
        return;
    }
    
}