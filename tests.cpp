#ifndef TESTS_H
#define TESTS_H

#include "recordtype.h"
#include "database.h"
#include "tester.h"

/**
 * @brief Function that tests functionality of the testRecordType
 * 
 */
void testRecordType() {
    // A constant recordtype
    const char* constantRTArgs[6] = {"age", "smallint", "name", "char(25)", "salary", "integer"};
    RecordType* constantRT = createRecordType("name", 6, constantRTArgs);
    
    // A variable recordtype 
    const char* pointerAndVariableRTArgs[4] = {"variablefield", "varchar(25)", "pointer", "pointer"};
    RecordType* pointerAndVariableRT = createRecordType("variablefield", 4, pointerAndVariableRTArgs);

    printf("Max size: %d\n", pointerAndVariableRT->maxSize);
    // Used for multiple assertions
    multiAssert("createRecordType", 28, 
        // Constant record type
        constantRT->maxSize == 31,
        constantRT->byteSizes[0] == 2,
        constantRT->byteSizes[1] == 25,
        constantRT->byteSizes[2] == 4,
        constantRT->isVariableLength == false,
        constantRT->byteOffsets[0] == 0,
        constantRT->byteOffsets[1] == 2,
        constantRT->byteOffsets[2] == 27,
        constantRT->fieldNameValueMap->at("age") == SmallIntType,
        constantRT->fieldNameValueMap->at("name") == CharType,
        constantRT->fieldNameValueMap->at("salary") == IntegerType,
        strcmp(constantRT->fieldNames[0], "age") == 0,
        strcmp(constantRT->fieldNames[1], "name") == 0,
        strcmp(constantRT->fieldNames[2], "salary") == 0,
        constantRT->fieldTypes[0] == SmallIntType,
        constantRT->fieldTypes[1] == CharType,
        constantRT->fieldTypes[2] == IntegerType,
        constantRT->numFields == 3,
        strcmp(constantRT->primaryField, "name") == 0,

        // Variable record type
        pointerAndVariableRT->maxSize == 25 + 8 + 6, // Because includes opening and closing characters
        pointerAndVariableRT->byteSizes[0] == 25,
        pointerAndVariableRT->byteSizes[1] == sizeof(void*),
        pointerAndVariableRT->isVariableLength == true,
        // We don't have to store byte offsets for variable length
        // entries because the variability of each size ruins it
        pointerAndVariableRT->fieldNameValueMap->at("pointer") == PointerType,
        pointerAndVariableRT->fieldNameValueMap->at("variablefield") == VarType,
        pointerAndVariableRT->fieldTypes[0] == VarType,
        pointerAndVariableRT->fieldTypes[1] == PointerType,
        pointerAndVariableRT->numFields == 2
    );
    
    multiAssert("getByteOffsetNumber", 5,
        getByteOffsetNumber(constantRT, "age") == 0,
        getByteOffsetNumber(constantRT, "name") == 2,
        getByteOffsetNumber(constantRT, "salary") == 27,
        getByteOffsetNumber(pointerAndVariableRT, "variablefield") == 0,
        getByteOffsetNumber(pointerAndVariableRT, "pointer") == 25
    );


    multiAssert("getFieldType and getFieldBytes", 10,
        getFieldType("smallint") == 1,
        getFieldType("integer") == 2,
        getFieldType("real") == 3,
        getFieldType("char(100)") == 4,
        getFieldType("varchar(78)") == 5,
        getFieldBytes("char(3212)") == 3212,
        getFieldBytes("varchar(78)") == 78,
        getFieldBytes("smallint") == 2,
        getFieldBytes("integer") == 4,
        getFieldBytes("real") == 4
    );

    multiAssert("convertStringToValue", 5, 
        *(short*) convertStringToValue(SmallIntType, "12") == 12,
        *(int*) convertStringToValue(IntegerType, "12000") == 12000,
        *(float*) convertStringToValue(RealType, "100.23") == (float) 100.23,
        strcmp((char*) convertStringToValue(CharType, "Neloy Kundu"), "Neloy Kundu") == 0,
        strcmp((char*) convertStringToValue(VarType, "Neloy Kundu "), "Neloy Kundu ") == 0
    );

    // We will create example records to test conversion functionality
    const char* constant_r1Args[3] = {"21", "Neloy Kundu", "120000"};
    char* constant_r1 = convertToDBRecord(constantRT, 3, constant_r1Args );
    
    const char* varRTArgs[6] = {"age", "integer", "name", "varchar(25)", "attractiveness", "real"};
    RecordType* variableRT = createRecordType("name", 6, varRTArgs);
    
    const char* var_r1Args[3] = {"21", "Neloy Kundu", "10.0"};
    char* variable_r1 = convertToDBRecord(variableRT, 3, var_r1Args);
    
    multiAssert("convertToDBRecord", 6,
        // Constant values
        * (short*) getFieldValue(constantRT, constant_r1, "age") == 21,
        strcmp((char*) getFieldValue(constantRT, constant_r1, "name"), "Neloy Kundu") == 0,
        * (int*) getFieldValue(constantRT, constant_r1, "salary") == 120000,
        
        // Variable-length values
        * (short*) getFieldValue(variableRT, variable_r1, "age") == 21,
        strcmp((char*) getFieldValue(variableRT, variable_r1, "name"), "Neloy Kundu") == 0,
        * (float*) getFieldValue(variableRT, variable_r1, "attractiveness") == (float) 10.0
    );
}

/**
 * @brief Function that tests functionality of the database's
 * block creation in multiple scenarios: creation of database primary block
 * table root blocks creation of the blocks for each table root block,
 * creation of blocks storing the entries, et cetera
 * 
 */
void testUnorderedDBOperations() {
    Database* db = createDatabase(UnorderedIndex, 512);

    // Used for database creation
    multiAssert("createDatabase", 1,
        db != NULL
    );

    // Do table creation and then do testing based on tables
    createTable(db, "student", "id", 6, "id", "smallint", "name", "char(25)", "age", "integer");
    void* tablePtr = getTableRootPtr(db, "student");
    multiAssert("createTable", 5,
        db->tableIndexMap->at("student") == 0,
        strcmp(db->tableRecordTypeMap->at("student")->fieldNames[0], "id") == 0,
        strcmp(db->tableRecordTypeMap->at("student")->fieldNames[1], "name") == 0,
        strcmp(db->tableRecordTypeMap->at("student")->fieldNames[2], "age") == 0,
        ((int*) tablePtr)[0] == 0,
        ((int*) tablePtr)[1] == 1
        
    );
}

int main ()  {
    testRecordType();
    testUnorderedDBOperations();
}

#endif