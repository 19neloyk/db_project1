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

        // Now we determine the maximum size that the record array can be
        if (recordType->isVariableLength == false) {
            // This max size is pretty straightforward: it's the byte offset
            // of the last element (counts number of bytes before the last
            // element) plus the byte size of the last element
            recordType->maxSize = recordType->byteOffsets[recordType->numFields - 1] + recordType->byteSizes[recordType->numFields - 1];
        } else {
            // This max size accounts for the fact that variable length
            // entries have to be separated and ended with a character
            // and that the variable length fields themselves have to
            // be separated and prepended with a character themselves

            // Initially the same size calculation as for the non
            // variable length type
            int maxSize = recordType->byteOffsets[recordType->numFields - 1] + recordType->byteSizes[recordType->numFields - 1];
            
            // Number of variable-length fields
            int numVariates = 0;
            for (int i = 0 ; i < recordType->numFields ; i ++) {
                if (recordType->fieldTypes[i] == 5) {
                    numVariates ++;
                }
            }

            // Add opening and closing symbols (~) for each variable-sized field
            // to max size
            maxSize += (2 * numVariates);

            // Add opening and closing symbols (%) for each entry to max size
            maxSize += 2;

            recordType->maxSize = maxSize;
        }


        return recordType;
    }
}

int getByteOffsetNumber(RecordType* rt, string field) {
    int fieldIndex = (*rt->fieldNameMap).at(field);
    return rt->byteOffsets[fieldIndex];
}

int getFieldType(char* typeString) {
    // Convert typeString to string type since it's easier to find
    // substrings within
    string convertedString(typeString);

    if (convertedString.find("smallint")  != string::npos) {
        // Small int case
        return 1;
    } else if (convertedString.find("integer")  != string::npos) {
        // Integer case
        return 2;
    } else if (convertedString.find("real")  != string::npos) {
        // Real int case
        return 3;
    } else if (convertedString.find("varchar")  != string::npos) {
        // Var char case; we do this before char because doing a 'string.find'
        // operation on a "varchar" would yield true
        return 5;
    } else if (convertedString.find("char")  != string::npos) {
        // Char case
        return 4;
    }

    // Case where this is not a valid type string
    return -1;
}

int getFieldBytes(char* stringedType) {
    // Pointer detailing when the integer starts
    char* intStart = stringedType;
    
    // Wait for pointer to get to '(' character, for example,
    // in "VARCHAR(10)" or "CHAR(15)"
    while (*intStart != '(') {
        intStart++;
    }

    // Go to the start of the integer (which starts right after the
    // '(' character)      
    intStart ++;
    
    // Now we can parse the integer with the atoi function since
    // intStart pointer is at the beginning of the integer
    return atoi(intStart);
}

char* convertToDBRecord(RecordType* rt, int length, ...) {
    // Make sure the number of arguments is the same
    if (rt->numFields != length/2) {
        return NULL;
    }

    // Iterate over values of a new record type
    va_list args;
    va_start(args, length);

    // The character string we will end up returning
    char* dbRecord = (char*) malloc(rt->maxSize);
    char* currentLocation = dbRecord;

    // Variable-length entry case --> opening '%'
    if (rt->isVariableLength) {
        *currentLocation = '%';
    }

    for (int i = 0; i < length ; i ++) {
        int fieldType = rt->fieldTypes[i];
        int byteLimit = rt->byteSizes[i];
        char* fieldValue = va_arg(args, char*);
        void* convertedValue = convertStringToValue(fieldType, fieldValue);
        if (convertedValue == NULL) {
            return NULL;
        }
        char* serializedValue = convertValueToType(fieldType, fieldValue, byteLimit);
        if (serializedValue == NULL) {
            return NULL;
        }

        // Converted fieldValue to serialized char arr that will
        // be stored as a record

        // Opening '~' character in case this is a variable length
        // char (fieldType of 5)
        if (fieldType == 5) {
            *currentLocation = '~';
            currentLocation ++;
        }

        // Note that serializedValue is the current field's value
        // We add this field to our array of bytes
        strcpy(currentLocation, serializedValue);
        
        // Move current location forward by the length of the current
        // field value so as to append the next field value correctly
        currentLocation += strlen(serializedValue);

        // Closing '~' character in case this is a variable length
        // char (fieldType of 5)
        if (fieldType == 5) {
            *currentLocation = '~';
            currentLocation ++;
        }
    }

    // Variable-length entry case --> closing '%'
    if (rt->isVariableLength) {
        *currentLocation = '%';
    }

    // At this point, our dbRecord character array can act as our record.
    // However, we have to "trim" our dbRecord in case it is variable length.
    // This is because if our record is variable-length, then it might necessarily
    // be the same size as we allocated to the dbRecord array. So we have to 
    // account for this

    if (rt->isVariableLength) {
        // Get EXACT length of the inhabited part of the dbRecord array
        // We do this by subtracting the location between the end of the
        // inhabited part of dbRecord (currently stored in the
        // currentLocation array)
        ptrdiff_t actualLength = currentLocation - dbRecord;
        char* tightDBRecord = (char*) malloc(actualLength);
        
        // Now we copy the elements from the original dbRecord to the
        // smaller variable-adjusted tightDBRecord
        char* originalPtr = dbRecord;
        char* tightPtr = tightDBRecord;
        for (int i = 0 ; i < actualLength ; i ++) {
            *tightPtr = *originalPtr;
            tightPtr ++;
            originalPtr++;
        }

        return tightDBRecord;
    } else {    // Don't need the else, but added for clarity to see the two situations
        return dbRecord;
    }


}



void* convertStringToValue(int type, char* valueString) {
    switch (type) {
        case 1: // Smallint type
            short* converted = (short*) malloc (sizeof(short));
            *converted = (short) stoi(valueString);
            break;
        case 2: // Integer type
            int* converted = (int*) malloc (sizeof(int));
            *converted = (int) stoi(valueString);
            break;
        case 3: // Real type
            float converted = stof(valueString);
            break;
        case 4: // Char(n) type
            return valueString;
            break;
        case 5: // Varchar(n) type
            return valueString;
            break;
        default:
            // Either type is 0 (which is a pointer, and we're never going to
            //read that as a string or it is invalid (not in the range [0, 5])
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
