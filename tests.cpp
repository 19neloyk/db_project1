#ifndef RECORDTYPE_TESTS_H
#define RECORDTYPE_TESTS_H

#include "recordtype.h"
#include "tester.h"

/**
 * @brief Function that tests functionality of the testRecordType
 * 
 */
void testRecordType() {
    // A constant recordtype
    RecordType* constantRT = createRecordType("name", 9, "age", SmallIntType, 0, "name", CharType, 25, "salary", IntegerType, 0);
    
    // A variable recordtype 
    RecordType* pointerAndVariableRT = createRecordType("variablefield", 6, "variablefield", VarType, 8, "pointer", PointerType, 0);

    printf("%s\n", constantRT->fieldNames[2]);

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
        pointerAndVariableRT->maxSize == sizeof(void*) + 8 + 6,
        pointerAndVariableRT->byteSizes[0] == 8,
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
        getByteOffsetNumber(pointerAndVariableRT, "pointer") == 8
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
    char* constant_r1 = convertToDBRecord(constantRT, 3, "21", "Neloy Kundu", "120000");
    void* deserialized_1 = getFieldValue(constantRT, constant_r1, "age");
    void* deserialized_2 = getFieldValue(constantRT, constant_r1, "name");
    void* deserialized_3 = getFieldValue(constantRT, constant_r1, "salary");

    //TODO: Have to focus on the most effective way to deserialize and
    // serialize data; must consider overarching design decisions
    /*
    multiAssert("convertToDBRecord",0

    );

    multiAssert("stringedType",0

    );
    */
}

int main()  {
    testRecordType();
}

#endif