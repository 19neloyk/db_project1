#include "database.h"
using namespace std;

Database* createDatabase(IndexType t, int blocksize) {
    Database* db = (Database*) malloc (sizeof(Database));
    db->type = t;
    db->numTables = 0;
    db->blockSize = blocksize;
    db->dbPrimaryBlock = (void**) malloc(db->blockSize);
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

void* getTableRootPtr(Database* db, const char* table_name) {
    int index = db->tableIndexMap->at(table_name);
    void* tablePtr = ((void**) db->dbPrimaryBlock)[index];
    return tablePtr;
}

RecordType* getTableRecordType(Database* db, const char* table_name) {
    return db->tableRecordTypeMap->at(table_name);
}

template<typename... Args>
void insert(Database* db, const char* table_name, int length, Args&&... args) {
    void* tableRootPtr =  getTableRootPtr(db, table_name);
    RecordType* rt = getTableRecordType(db, table_name);
    char* record = convertToDBRecord(rt, length, args);
    if (db->type == 0) {
        insert_unordered();
    } else if (db->type == 1) {
        printf("This index has not been implemented yet.\n");
    } else {
        printf("Invalid type.\n");
    }

}

void insert_unordered (Database* db, RecordType* rt, void* tableRootPtr, char* record) {
    // The array of blocks within this table root block
    int numEntries = ((int*) tableRootPtr)[0];
    int numBlocks = ((int*) tableRootPtr)[1];
    void** blockArr = (void**) (((int*) tableRootPtr) + 2);

    // If the record is of variable length
    if (rt->isVariableLength) {
        char* curBlock = (char*) blockArr[numBlocks];
        
        // In case the block has no entries
        if (*curBlock != '%') {
            strcpy(curBlock, record);
            printf("Successful insert! \n");
            return;
        }

        // Go to the last record of the last block by going from
        // the end of the block to the first '%' encountered
        // (which is what would end the last entry in this block)
        char* endOfBlock = curBlock + (db->blockSize - 1);
        char* endOfRecords = endOfBlock;
        while (*endOfRecords != '%') {
            endOfRecords --;
        }
        
        // Now check if we can fit this record into the current block,
        // otherwise, add another block to the table and rerun the insert operation!
        ptrdiff_t remainingBlockSpace = endOfBlock - endOfRecords;


        if (remainingBlockSpace < strlen(record)) {
            strcpy(endOfRecords + 1, record);
            printf("Successful insert! \n");
            return;
        } else {
            printf("Adding another block to insert!\n");
            // If we can add another block,
            // then we are fine and rerun the insert,
            // otherwise, that means we have no more space within our tables!
            if (addBlock(db, tableRootPtr)) {
                insert_unordered(db, rt, tableRootPtr, record);
            } else {
                printf("No more space: we can't add another block, sadly... Insert failed\n");
                return ;
            }
        } 
    } else {
        // Case where the record is not variable length; it is constant in length
        // Calculate exactly where the block would go within the last block;
        int bfr = (db->blockSize - sizeof(int) - sizeof(int))/rt->maxSize;
        int position = bfr % (numEntries) == 0;

        // Add to current block as long as current block is not full
        // (or we just get a 0 because no entries have been inserted)
        if (numEntries == 0 || position != 0) {
            char* entryPosition = ((char*) blockArr) + (position * rt->maxSize);
            strcpy(entryPosition, record);
            printf("Successful insert! \n");
            return;
        } else {
            if (addBlock(db, tableRootPtr)) {
                insert_unordered(db, rt, tableRootPtr, record);
            } else {
                printf("No more space: we can't add another block, sadly... Insert failed\n");
                return ;
            }
        }
    }
}

