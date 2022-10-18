#ifndef RECORDTYPE_TESTS_H
#define RECORDTYPE_TESTS_H

#include "recordtype.h"
#include "tester.h"

bool testCreateRecordType() {

    // A constant recordtype
    RecordType* constantRT = createRecordType(3, "age", SmallIntType, 0, "name", CharType, 25, "salary", IntegerType, 0);
    // A variable recordtype 

    // An index recordtype (it's just a name and a pointer stored)

    // Used for multiple assertions
    return multiAssert("testCreateRecordType", 1, 
        1 == 2
    );
}

bool testGetByteOffsetNumber() {


    // Used for multiple assertions
    return multiAssert("testGetByteOffsetNumber", 1, 
        1 == 1
    );
}

bool testGetFieldType() {

    // Used for multiple assertions
    return multiAssert("testGetFieldType", 1, 
        1 == 1
    );
}

bool testCheckType() {

    // Used for multiple assertions
    return multiAssert("testCheckType", 1, 
        1 == 1
    );
}

bool testConvertToDBRecord() {

    // Used for multiple assertions
    return multiAssert("testConvertToDBRecord", 1, 
        1 == 1
    );
}

bool testStringedType() {

    // Used for multiple assertions
    return multiAssert("testStringedType", 1, 
        1 == 1
    );
}


int main()  {
    testCreateRecordType();
}

#endif