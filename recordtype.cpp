#include "recordtype.h"


RecordType* createRecordType(int length, ...) {

    // Create the new record type
    RecordType* recordType = (RecordType*) malloc(sizeof(RecordType));

    // Create sizes for class array variables we have created
    recordType->numFields = length/3;

    recordType->fieldTypes = (int*) malloc (sizeof(recordType->numFields));
    recordType->byteSizes = (int*) malloc (sizeof(recordType->numFields));
    recordType->byteOffsets = (int*) malloc (sizeof(recordType->numFields));

    recordType->fieldNameMap = new map<string, int>();

    va_list args;
    va_start(args, length);

    string curName;
    int curType;
    int curN;

    recordType->isVariableLength = false;
    for (int i = 0; i < length ; i ++) {
        switch (i % 3) {
            case 0:
                curName = va_arg(args,char*);
                break;
            case 1:
                curType = va_arg(args, int);
                break;
            default  :
                curN = va_arg(args, int);

                recordType->fieldTypes[i/3] = curType;
                switch (curType) {
                    case 0:
                        recordType->byteSizes[i/3] = sizeof(void*);
                        break;
                    case 1:
                        recordType->byteSizes[i/3] = 2;
                        break;
                    case 2:
                        recordType->byteSizes[i/3] = 4;
                        break;
                    case 3:
                        recordType->byteSizes[i/3] = 4;
                        break;
                    case 4:
                        recordType->byteSizes[i/3] = curN;
                        break;
                    case 5:
                        recordType->byteSizes[i/3] = curN;
                        recordType->isVariableLength = false;
                        break;
                    default:
                        recordType->byteSizes[i/3] = -1;

                }

                if (i/3 == 0) {
                    recordType->byteOffsets[i/3] = 0;
                } else {
                    recordType->byteOffsets[i/3] = recordType->byteOffsets[(i/3) - 1] + recordType->byteSizes[(i/3) - 1];
                }
                (*recordType->fieldNameMap).insert({curName, curType});
        }

        return recordType;
    }
}

int getByteOffsetNumber(RecordType* rt, string field) {
    int fieldIndex = (*rt->fieldNameMap).at(field);
    return rt->byteOffsets[fieldIndex];
}

void* convertToRecord(int length, ...) {

}

bool checkType(RecordType* rt, int length, ...) {
    // Make sure the number of arguments is the same
    if (rt->numFields != length/2) {
        return false;
    }

    // Iterate over values of a new record type
    va_list args;
    va_start(args, length);

    for (int i = 0; i < length ; i ++) {
        int fieldType = rt->fieldTypes[i];
        int byteLimit = rt->byteSizes[i];
        char* fieldValue = va_arg(args, char*);
    }
    
}

void* convertStringToValue(int type, char* valueString) {
    switch (type) {
        case 0: // Pointer type

            break;
        case 1: // Smallint type
            return (void*) short* converted = (short*) malloc ();
            break;
        case 2: // Integer type

            break;
        case 3: // Realint type

            break;
        case 4: // Char(n) type

            break;
        case 5: // Varchar(n) type

            break;
        default:
            // Invalid type
            return NULL;
    }
}
    

// Helper function to convert element into a character array
// we can store in our database
char* convertValueToType (int type, void* roughValue, int byteLimit) {
    switch (type) {
        case 0: // Pointer type 
            char* serialized = (char*) roughValue;
            return serialized;
            break; 
        case 1: // smallint type - will be a short
            char* serialized = (char*) roughValue;
            return serialized;
            break; 
            break;
        case 2: // integer type - will be an int
            char* serialized = (char*) roughValue;
            return serialized;
            break;
        case 3: // real type - will be a double
            char* serialized = (char*) roughValue;
            return serialized;
            break;
        case 4: // char(n) type; returns array of exactly n characters; padded with spaces
            // Cast to char* type
            char* serialized = (char*) roughValue;

            // Make sure length of string is not greater than n (the byteLimit)
            if (strlen(serialized) > byteLimit) {
                return NULL;
            }
            // Get string we will return, make it n spaces
            // so we don't have to worry about padding 
            char* fullString = (char*) malloc(byteLimit);
            for (int i = 0 ; i < byteLimit ; i ++) {
                *fullString = ' ';
            }

            // Copy initial string to space padded string
            strcpy(fullString, serialized);
            
            return fullString;
            break;
        case 5: // varchar(n) type
            // Cast to char* type
            char* serialized = (char*) roughValue;

            // Make sure length of string is not greater than n (the byteLimit)
            if (strlen(serialized) > byteLimit) {
                return NULL;
            }

            // Worry about separation with other characters (i.e. % or ~)
            // at the block level
            return serialized;
            break;
        default:
            // Invalid type
            return NULL;
            break;

    }
}



string stringedType(int n) {
    switch (n) {
        case 0:
            return "POINTER TYPE";
            break;
        case 1:
            return "SMALLINT TYPE";
            break;
        case 2:
            return "INTEGER TYPE";
            break;
        case 3:
            return "REAL TYPE";
            break;
        case 4:
            return "CHAR(N) TYPE";
            break;
        case 5:
            return "VARCHAR(N) TYPE";
            break;
        default:
            return "INVALID TYPE";
    }                
