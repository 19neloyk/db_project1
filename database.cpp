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
    return true;
}

void createTable(Database* db, const char *table_name, const char *primary_key, int length, ...) {
    // Check if there is already a table with the specified name
    string tableName = string(table_name);
    if (db->tableIndexMap->find(tableName) != db->tableIndexMap->end()) {
    // not found case
        printf("Table '%s' already exists, please use another name\n", table_name);
        return;
    } 

    const char* rtArgs[length];
    va_list args;
    va_start(args, length);
    for (int i = 0 ; i < length ; i ++) {
        rtArgs[i] = va_arg(args, char*);
    }
    // Create the specified record type and add into record type map
    RecordType* rt = createRecordType(primary_key, length, rtArgs);
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

    if (!addBlock(db, tableRootBlockPtr)) {
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
            ((int*) tableRootPtr)[0] ++;
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
                return;
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

void insert(Database* db, const char* table_name, int length, ...) {
    void* tableRootPtr =  getTableRootPtr(db, table_name);
    RecordType* rt = getTableRecordType(db, table_name);

    const char* rtArgs[length];
    va_list args;
    va_start(args, length);
    for (int i = 0 ; i < length ; i ++) {
        rtArgs[i] = va_arg(args, char*);
    }

    char* record = convertToDBRecord(rt, length, rtArgs);
    if (db->type == 0) {
        insert_unordered(db, rt, tableRootPtr, record);
    } else if (db->type == 1) {
        printf("This index has not been implemented yet.\n");
    } else {
        printf("Invalid type.\n");
    }
}

int select (Database* db, const char *tableName, int length, ...) {
    // First extract selected fields and where clause
    const char* rtArgs[length];
    va_list args;
    va_start(args, length);

    RecordType* rt = db->tableRecordTypeMap->at(tableName);
    
    // Selected fields (separated by ', ': comma and a space)
    // Only support one condition right now
    char *selectedFields, *condition;
    selectedFields = va_arg(args, char*);
    condition = va_arg(args, char*);
    
    // Parse for the selectedFields
    // Get number of commas first
    int numFieldsWanted = 1;
    int selFieldLen = strlen(selectedFields);
    for (int i = 0 ; i < selFieldLen ; i ++) {
        if (selectedFields[i] == ',') {
            numFieldsWanted ++;
        }
    }

    // Now make an array of stringed fields
    // Let each element of the array be a pointer
    // to the first character of the field
    char* fieldNameStarts[numFieldsWanted];
    fieldNameStarts[0] = selectedFields;
    
    int curIndex = 1;
    for (int i = 1 ; i < selFieldLen ; i ++) {
        if (selectedFields[i] == ',') {
            // Do this so that we can use strcpy later
            selectedFields[i] = '\0';

            // First character of field will be located after
            // ', '
            fieldNameStarts[curIndex] = selectedFields + 2;
            curIndex ++;
        }
    }

    // Currently, if every field is wanted, then fieldsWanted[0]
    // will be "*", otherwise it will be a list


    // We will now scan what the condition is
    // leftArgument represents a field and 
    // rightArgument represents a values
    char leftArgument[100], rightArgument[100];
    char op;
    sscanf(condition, "%s %c %s", leftArgument, op, rightArgument);
    string leftArg = string(leftArgument);
    
    // Now go to the table and iterate over its entries
    void* tableRootBlockPtr = getTableRootPtr(db, tableName);
    int numEntries = ((int*) tableRootBlockPtr)[0];
    int numBlocks = ((int*) tableRootBlockPtr)[1];
    void** blockArr = (void**) (((int*) tableRootBlockPtr) + 2);

    int numMatches = 0;
    
    // Iterate over each block
    for (int i = 0; i < numBlocks; i ++) {

        void* curBlockPointer = blockArr[i];
        
        // Iterate over each element in the current block
        // First deal with the non-variable case, then deal
        // with the variable case

        // Non-variable case
        if (rt->isVariableLength) {
            // Num of blocks in our current array
            int elementCountCurBlock = db->blockSize/rt->maxSize;

            // Element count in case where this is the last block
            if (i == numBlocks - 1) {
                elementCountCurBlock = numEntries % numBlocks;
            }
            

            for (int j = 0 ; j < elementCountCurBlock ; j ++) {
                // Get pointer to current record within the block
                char* recordPointer = ((char*) curBlockPointer) + (j * rt->maxSize);
                char serializedRecord[rt->maxSize]; 
                memcpy(serializedRecord, recordPointer, rt->maxSize);
                int matchResult = isMatchingRecord(rt, leftArgument, op, rightArgument, serializedRecord);
                if (matchResult == 1) {
                    numMatches ++;
                    printf("–––––––––");
                    for (int k = 0 ; k < numFieldsWanted; k ++) {
                        printFieldValue(rt, serializedRecord, fieldNameStarts[k]);
                    }
                    printf("–––––––––");
                }
                if (matchResult == -1) {
                    printf("ERROR with figuring out if this entry is a match\n");
                }
            }

        // Now we deal with the variable case
        } else {

        }
    }
}
