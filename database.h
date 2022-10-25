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
 * @param tableRootPtr the table from which we get the additional entries
 * @return bool indicates whether the addition operation was successful or not
*/
bool addBlock(void* tableRootPtr);


/**
 * @brief Get a pointer for a certain table's root block
 * according to its name
 * @param name 
 * @return void* get the address to a specified table
 */
void* getTableRootPtr(char* name);

/**
 * @brief Get a the record type for a certain table based on
 * its name
 * @param name 
 * @return void* the record type of the specified table
 */
void* getTableRecordType(char* name);

template<typename... Args>void createTable(Database* db, const char *table_name, const char *primary_key, int length, Args&& ...);
void select(Database* db, const char *table_name, int length, ...);
void insert(Database* db, const char *table_name, int length, ...);
void update(Database* db, const char *table_name, int length, ...);

#endif
