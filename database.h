#ifndef DATABASE_H
#define DATABASE_H
#include "recordtype.h"
#include "block.h"
#include <string.h>
using namespace std;

class Database {
    RecordType* primaryBlockType;
    Block* primaryBlock;

    Database* createDatabase();

    void processQuery(Database* db, string query);

    void create_table(Database* db, const char *table_name, const char *primary_key, int length, ...);
    void select(Database* db, const char *table_name, int length, ...);
    void insert(Database* db, const char *table_name, int length, ...);
    void update(Database* db, const char *table_name, int length, ...);

};

#endif
