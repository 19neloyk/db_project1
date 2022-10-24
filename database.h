#ifndef DATABASE_H
#define DATABASE_H
#include "recordtype.h"
#include "block.h"
#include <string.h>
using namespace std;

enum IndexType{
    UnorderedIndex,
    OrderedIndex,
    HashedIndex
};

class Database {

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
     * 
     * points to void* 's that each represent a table root;
     * each table's root block; in each table, which is a
     * block of char where 
     */
    void** tables;

    /**
     * @brief Maps table name to index in table's pointer array above
     * 
     */
    map<string, int>* tableIndexMap;

    /**
     * @brief Maps table name to corresponding record type
     * 
     */
    map<string, RecordType*>* tableRecordTypeMap;

    /**
     * @brief Get a pointer for a certain table's root block
     * according to its name
     * @param name 
     * @return void* 
     */
    void* getTableRoot(char* name);

    /**
     * @brief Get a the record type for a certain table based on
     * its name
     * @param name 
     * @return void* s
     */
    void* getTableRecordType(char* name);


    Database* createDatabase();

    void processQuery(Database* db, string query);

    void create_table(Database* db, const char *table_name, const char *primary_key, int length, ...);
    void select(Database* db, const char *table_name, int length, ...);
    void insert(Database* db, const char *table_name, int length, ...);
    void update(Database* db, const char *table_name, int length, ...);

};

#endif
