#ifndef TESTER_H
#define TESTER_H

#include <stdarg.h>
#include <stdio.h>
#include <string.h>


/**
 * @brief Tests multiple conditions
 * 
 * @param testName The name of the tests 
 * @param numTests The number of tests
 * @param ... 
 * @return true if every test passes
 * @return false if at least one test fails
 */
bool multiAssert(const char* testName, int numTests, ...);

#endif