#include "recordtype.h"
using namespace std;


void* convertStringToValue(int type, const char* valueString) {
    if (type == 1) {
        short* converted = (short*) malloc (sizeof(short));
        *converted = (short) stoi(valueString);
        return converted;
    } else if (type == 2) {
        int* converted = (int*) malloc (sizeof(int));
        *converted = (int) stoi(valueString);
        return converted;   
    } else if (type == 3) {
        float* converted = (float*) malloc (sizeof(float));
        *converted = (float) stof(valueString);
        return converted;
    } else if (type == 4 || type == 5) {
        char* converted = (char*) malloc (strlen(valueString));
        strcpy(converted, valueString);
        return converted;
    }

    // Case where the value string does not match any type
    return NULL;
}
    

// Helper function to convert element into a character array
// we can store in our database
char* convertValueToType (int type, void* roughValue, int byteLimit) {
    // Serialization of first 4 types is straight forward
    // (already stored in a memory size we've defined,
    // so the casting is plain)

    if (type == 1) {
        char* serialized = (char*) malloc (byteLimit);
        *(short *) serialized  = * ((short*) roughValue);
        return serialized;
    } else if (type == 2) {
        char* serialized = (char*) malloc (byteLimit);
        printf("Int: %d\n", *((int*) roughValue));
        *(int *) serialized  = * ((int*) roughValue);
        return serialized;
    } else if (type == 3) {
        char* serialized = (char*) malloc (byteLimit);
        *(float *) serialized  = * ((float*) roughValue);
        return serialized;
    }

    switch (type) {
        case 4: // char(n) type; returns array of exactly n characters; padded with spaces
            // Keep in separate scope so as to not have any
            // naming problems with the serialized variable
            {
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
            }
        case 5: // varchar(n) type
            {
                // Cast to char* type
                char* serialized = (char*) roughValue;

                // Make sure length of string is not greater than n (the byteLimit)
                if (strlen(serialized) > byteLimit) {
                    return NULL;
                }

                // Worry about separation with other characters (i.e. % or ~)
                // at the block level
                return serialized;
            }
        default:
            // Invalid type
            return NULL;
    }
}

int getByteOffsetNumber(RecordType* rt, string field) {
    int fieldIndex = rt->fieldNameIndexMap->at(field);
    return rt->byteOffsets[fieldIndex];
}

int getFieldType(const char* typeString) {
    // Convert typeString to string type since it's easier to find
    // substrings within
    string convertedString(typeString);

    if (convertedString.find("smallint") != string::npos) {
        // Small int case
        return 1;
    } else if (convertedString.find("integer") != string::npos) {
        // Integer case
        return 2;
    } else if (convertedString.find("real") != string::npos) {
        // Real int case
        return 3;
    } else if (convertedString.find("varchar") != string::npos) {
        // Var char case; we do this before char because doing a 'string.find'
        // operation on a "varchar" would yield true
        return 5;
    } else if (convertedString.find("char") != string::npos) {
        // Char case
        return 4;
    }

    // Case where this is not a valid type string
    return -1;
}

int getFieldBytes(const char* stringedType) {
    
    // First deal with already preset size types (1 to 3) that
    // the user can input:  smallint, integer, and real
    int fieldType = getFieldType(stringedType);

    if (fieldType == 1) {
        return 2;
    } else if (fieldType == 2) {
        return 4;
    } else if (fieldType == 3) {
        return 4;
    }

    // Finally, we can deal with the types that do not have
    // preset sizes: char and varchar

    // Pointer detailing when the integer starts
    const char* intStart = stringedType;
    
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
    if (rt->numFields != length) {
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
        char* serializedValue = convertValueToType(fieldType, convertedValue, byteLimit);
        if (serializedValue == NULL) {
            return NULL;
        }

        // Converted fieldValue to serialized char arr that will
        // be stored as a record
        // Opening '~' character in case so that we can easily
        // find the value for each field
        if (rt->isVariableLength) {
            *currentLocation = '~';
            currentLocation ++;
        }

        // Note that serializedValue is the current field's value
        // We add this field to our array of bytes
        strcpy(currentLocation, serializedValue);
        
        // Move current location forward by the length of the current
        // field value so as to append the next field value correctly
        if (fieldType != VarType) {
            currentLocation += byteLimit;
        } else {
            currentLocation += strlen(serializedValue);
        }

        // Closing '~' character in variable length for same reason
        // above
        if (rt->isVariableLength) {
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
}

RecordType* createRecordType(const char* primaryKey, int length, ...) {

    // Create the new record type
    RecordType* recordType = (RecordType*) malloc(sizeof(RecordType));
    // Update primary key for this record type
    recordType->primaryField = primaryKey;

    // Create sizes for class array variables we have created
    recordType->numFields = length/3;

    // Organizational structures that will help us immensely
    recordType->fieldTypes = (int*) malloc (sizeof(int) * recordType->numFields);
    recordType->byteSizes = (int*) malloc (sizeof(int) * recordType->numFields);
    recordType->byteOffsets = (int*) malloc (sizeof(int) * recordType->numFields);
    recordType->fieldNames = (char**) malloc (sizeof(char*) * recordType->numFields);
    
    recordType->fieldNameValueMap = new map<string, int>();
    recordType->fieldNameIndexMap = new map<string, int>();

    va_list args;
    va_start(args, length);

    char* name;
    int curType;
    int curN;

    recordType->isVariableLength = false;
    for (int i = 0; i < length ; i ++) {
        if (i % 3 == 0) {
            name = va_arg(args,char*);
        } else if (i % 3 == 1) {
            curType = va_arg(args, int);
        } else {
            curN = va_arg(args, int);
            
            // Used to create a new instance to 
            char* allocatedFieldName = (char *) malloc (strlen(name) + 1);
            strcpy(allocatedFieldName, name);
            recordType->fieldNames[i/3] = allocatedFieldName;

            // Use string because we want to leverage its ease of use in the
            // maps we use for organizational efficiency
            string curName = name;
            recordType->fieldNameIndexMap->insert(make_pair(curName, i/3));

            recordType->fieldTypes[i/3] = curType;
            int byteSize = 0;
            if (curType == 0) {
                byteSize = sizeof(void*);
            } else if (curType == 1) {
                byteSize = 2;
            } else if (curType == 2 || curType == 3) {
                byteSize = 4;
            } else if (curType == 4) {
                byteSize = curN;
            } else if (curType == 5) {
                byteSize = curN;
                recordType->isVariableLength = true;
            } else {
                byteSize = -1;
            }
            
            (recordType->byteSizes)[i/3] = byteSize;

            if (i/3 == 0) {
                recordType->byteOffsets[i/3] = 0;
            } else {
                recordType->byteOffsets[i/3] = recordType->byteOffsets[(i/3) - 1] + recordType->byteSizes[(i/3) - 1];
            }
            recordType->fieldNameValueMap->insert(make_pair(curName, curType));
        }

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
        
        

        // Add opening and closing symbols (~) for each field
        // to max size
        maxSize += (2 * recordType->numFields);

        // Add opening and closing symbols (%) for each entry to max size
        maxSize += 2;

        recordType->maxSize = maxSize;
    }

    return recordType;
}                

void printFieldValue (RecordType* rt, void* deserializedValue, const char* fieldName) {
    int fieldValue = rt->fieldNameValueMap->at(fieldName);
    if (fieldValue == SmallIntType) {
        printf("%s %d\n", fieldName, *(short*) deserializedValue);
    } else if (fieldValue == IntegerType) {
        printf("%s %d\n", fieldName, *(int*) deserializedValue);
    } else if (fieldValue == RealType) {
        printf("%s %f\n", fieldName, *(float*) deserializedValue);
    } else if (fieldValue == CharType || fieldValue == VarType) {
        printf("%s %s\n", fieldName, (char*) deserializedValue);
    } else {
        printf("%s is an invalid field\n", fieldName);
    }
}

void* getFieldValue (RecordType* rt, char* serializedEntry, const char* fieldName) {
    // Deal with variable_length and non-variable-length entry case separately

    // Variable length case
    if (rt->isVariableLength) {
        int fieldIndex = rt->fieldNameIndexMap->at(fieldName);
        char* startPtr = serializedEntry;
        
        // We want the number of ~'s to be 2 * fieldIndex + 1
        int fieldDividerCount = 0;
        while (fieldDividerCount != 2 * fieldIndex + 1) {
            if (*startPtr == '~') {
                fieldDividerCount ++;
            }
        }

        // Now start pointer should be located at the start of he field 
        // Let us get the size of the field entry; accomodate the same
        // code for both variable and constant-sized fields
        int fieldSize = 0;
        int i = 0;
        while (*(startPtr + i) != '~') {
            i++;
            fieldSize ++;
        }

        char* deserialized = (char*) malloc (fieldSize + 1);
        memcpy(deserialized, startPtr, fieldSize + 1);
        return deserialized;
    }

    // Constant length case 
    int fieldByteOffset = getByteOffsetNumber(rt, fieldName);
    char* startPtr = serializedEntry + fieldByteOffset;

    int fieldSize = getFieldBytes(fieldName);
    char* deserialized = (char*) malloc (fieldSize + 1);
    memcpy(deserialized, startPtr, fieldSize + 1);

    printFieldValue(rt, deserialized, fieldName);
    
    return deserialized;
}
