#include "tester.h"


bool multiAssert(const char* testName, int numTests, ...) {
    va_list args;
    va_start(args, numTests);
    
    bool allTestsPassed = true;

    printf("------------------------------\n");
    printf("TESTS FOR: ");
    printf("%s", testName);
    printf("\n");

    for (int i = 0; i < numTests ; i ++) {
        if (va_arg(args, int) == 0) {
            printf("TEST #%d FAILED\n", i + 1);
            allTestsPassed = false;
        } else {
            printf("TEST #%d PASSED\n", i + 1);
        }
    }
    if (allTestsPassed) {
        printf("ALL TESTS PASSED\n");
    } else {
        printf("ALL TESTS FAILED\n");
    }

    printf("------------------------------\n");
    return allTestsPassed;
}