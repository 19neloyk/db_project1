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
                    default:
                        recordType->byteSizes[i/3] = curN;

                }

                if (i/3 == 0) {
                    recordType->byteOffsets[i/3] = 0;
                } else {
                    recordType->byteOffsets[i/3] = recordType->byteOffsets[(i/3) - 1] + recordType->byteSizes[(i/3) - 1];
                }

                (*recordType->fieldNameMap).insert({curName, curType});
                
        }
    }
}

int getByteOffsetNumber(RecordType* rt, string field) {

}

bool checkType(int length, ...) {
    
}

string stringedType(int n) {

}