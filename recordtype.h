#ifndef RECORDTYPE_H
#define RECORDTYPE_H
#include <string>
#include <map>
#include <stdio.h>
#include <stdarg.h>

using namespace std;

enum FieldType {
    PointerType,
    SmallIntType,
    IntegerType,
    RealType,
    CharType,
    VarType
};

struct RecordType {
    // Array of field names as they appear in a record, i.e.
    // fieldname_0, fieldname_1, fieldname_2, ... ,fieldname_k
    char** fieldNames;

    // Map from name of field to type value of field;
    // used for records in the table and records for each table
    // in primary database block
    map<string, int>* fieldNameValueMap;

    // Map from name of field to index in fieldNames arr
    map<string, int>* fieldNameIndexMap;

    // Number of fields
    int numFields;

    // Array of fields (indicated by numbers) for this RecordType
    // 0 - pointer: sizeof(void*) 
    // 1 - smallint: 2 bytes
    // 2 - integer: 4 bytes
    // 3 - real: 4 bytes
    // 4 - char(n): n bytes
    // 5 - varchar(n): n bytes
    int* fieldTypes;

    // Denotes the primary field
    const char* primaryField;

    // Indicates byte sizes of each entry of the fieldTypes;
    // this is in the same order as the 
    int* byteSizes;


    // byteOffsets[i] is how many byte offsets for the ith index;
    // This array is constructed upon creation of the record type
    int* byteOffsets;

    // Maximum possible size of a record
    int maxSize;

    // Whether there is a varchar field type within this record;
    // denotes that a record using this RecordType object is
    // variable in size
    bool isVariableLength;
};

/**
 * @brief Get the number of bytes offset by for a given field,
 * i.e. for records in block a, this entry might start at
 * index a_i, so a specified field value for this might start
 * at index a_{i + j}, where j is the byte offset number
 * returned from this function
 * 
 * @param rt the record type we would like to extract from
 * @param field the name of the field we want to get the
 * byte offset of
 * @return the byte offset number of the location of the given
 * field relative to the start of the record (so assuming
 * the record starts at index 0)
 */
int getByteOffsetNumber(RecordType* rt, string field);

/**
 * @brief Get the number of types in a string
 * 
 * @param stringedType 
 * @return int representing the variable type 
 */
int getFieldType(const char* typeString);

/**
 * @brief Get the number of bytes with the specified type string
 * i.e. "CHAR(15)" or "VARCHAR(10)"
 * 
 * @param stringedType 
 * @return int representing number of bytes 
 */
int getFieldBytes(const char* stringedType);

// Convert a series of arguments representing an entry into an
// actual record that can be put on memory
//NOTE: This is only for database entries, not Index entries!
void* convertToDBRecord(int length, ...);

// Get string version of type
string stringedType(int n); 

/**
 * @brief creates a record type for entries to be of
 * @param primaryKey the primary key of this record type
 * @param length the number of arguments
 * @param ellipses (...) of form (fieldName, fieldType, fieldN, fieldName,
 * fieldType, fieldN...), so triples (fieldN is 0 id fieldType is not char or
 * varchar)
 * @return a record type 
 */
RecordType* createRecordType( const char* primaryKey, int length, ...);



#endif