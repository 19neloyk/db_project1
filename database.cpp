#include "database.h"

Database* createDatabase(IndexType t, int blocksize) {
    Database* db = (Database*) malloc (sizeof(Database));
    db->type = t;
    db->numTables = 0;
    db->dbPrimaryBlock = (void**) malloc(blocksize);
    db->tableIndexMap = new map<string, int>();
    db->tableRecordTypeMap = new map<string, RecordType*>;
    return db;
}

template<typename... Args>void createTable(Database* db, const char *table_name, const char *primary_key, int length, Args... args) {
    // Check if there is already a table with the specified name
    string tableName = string(table_name);
    if (db->tableIndexMap->find(tableName) == db->tableIndexMap->end()) {
    // not found
        printf("Table '%s' already exists, please use another name\n", tableName);
        return;
    } 

    // Create the specified record type
    RecordType* rt = createRecordType(primary_key, length, args);
}