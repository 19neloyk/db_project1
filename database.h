#ifndef DATABASE_H
#define DATABASE_H
#include "recordtype.h"
#include <string.h>
#include <map>
#include <stdio.h>
#include <stdarg.h>

using namespace std;

enum IndexType {
    UnorderedIndex,
    OrderedIndex,
    HashedIndex
};

struct Database {
    /**
     * @brief Denotes the indexing structure that will
     * be used either unordered, ordered, or hashed
     * 
     */
    IndexType type;

    /**
     * @brief represents the number of tables
     * 
     */
    int numTables;

    /** @brief list of tables
     * points to void* 's that each represent a table root;
     * each table's root block; in each root block is an
     * array of 
     */
    void** dbPrimaryBlock;

    /**
     * @brief the size in bytes of allocated blocks
     * for this database
     * 
     */
    int blockSize;

    /**
     * @brief Maps table name to index in dbPrimaryBlock's
     * pointer array above
     */
    map<string, int>* tableIndexMap;

    /**
     * @brief Maps table name to corresponding record type
     * 
     */
    map<string, RecordType*>* tableRecordTypeMap;
};

/** 
 * @brief Create a new Database struct
 * @param t the type of indexing in the
 * database that we will use
 * @param blocksize indicates the block
 * sizes that we will use with this db
 * 
 * @return a newly allocated database 
 */
Database* createDatabase(IndexType t, int blocksize);

/**
 * @brief Add an additional block (for entries)
 * to the specified table by pointer access
 * 
 * @param db the database that we are working in
 * @param tableRootPtr the table from which we get the additional entries
 * @return bool indicates whether the addition operation was successful or not
*/
bool addBlock(Database* db, void* tableRootPtr);

/**
 * @brief Get a pointer for a certain table's root block
 * according to its name
 * @param db the database that we are accessing
 * @param name name of table we are looking for
 * @return void* get the address to a specified table
 */
void* getTableRootPtr(Database* db, const char* name);

/**
 * @brief Get a the record type for a certain table based on
 * its name
 * @param name 
 * @return RecordType* the record type of the specified table
 */
RecordType* getTableRecordType(Database* db, char* table_name);

void createTable(Database* db, const char *table_name, const char *primary_key, int length, ...);

/**
 * @brief Get pointers to each record 
 * 
 * @param db the database that we are pulling from
 * @param rt the record type of the records in the table
 * @param tableRootPtr pointer to the table root
 * @param condition the condition that we would like to evaluate for
 * @param totalEntries total number of entries in this table
 * @return char** 
 */
char** queriedRecords (Database* db, RecordType* rt, void* tableRootPtr, const char* condition, int totalEntries);


/**
 * @brief print matching records from the database
 * 
 * @param db the database we are working on
 * @param tableName name of the table
 * @param length how many arguments after
 * @param ... 
 * @return int - the number of matching elements
 */
int select(Database* db, const char *tableName, int length, ...);

/**
 * @brief insert a record into the database
 * 
 * @param db the database that we are working on
 * @param table_name name of the table
 * @param length how many arguments after
 * @param ... 
 */
void insert(Database* db, const char *table_name, int length, ...);


/**
 * @brief update an entry or multiple entries in the database
 * 
 * @param db the database that we are working on 
 * @param tableName name of the table
 * @param length how many arguments are coming after
 * @param int - the number of entries that we have updated
 */
int update(Database* db, const char *tableName, int length, ...);

#endif
