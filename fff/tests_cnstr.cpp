#include <cassert>
#include <cstring>
#include "fff.h"  
#include "tests_cnstr.h"

void runAllCnstr() {
    testDefaultConstructor();
    testConstructorFromCStr();
    testConstructorFromNullCStr();
    testConstructorFromCStrWithLength();
    testConstructorFromLengthAndChar();
    testDestructorReleasesMemory();
}

void testDefaultConstructor() {
    
    try {
        String str;
        assert(str.size() == 0);
        str.data();
        assert(false);
    }
    catch (const std::out_of_range&) {
    }
}

void testConstructorFromCStr() {
    const char* testStr = "Hello, World!";
    String str(testStr);
    assert(str.size() == strlen(testStr));
    assert(strcmp(str.data(), testStr) == 0); 
}

void testConstructorFromNullCStr() {
    String str(nullptr);
    assert(str.size() == 0);
    try {
        str.data();
        assert(false);
    }
    catch (const std::out_of_range&) {
    }
}

void testConstructorFromCStrWithLength() {
    const char* testStr = "Hello, World!";
    String str(testStr, 5); 
    assert(str.size() == 5);
    assert(strcmp(str.data(), "Hello") == 0); 
}

void testConstructorFromLengthAndChar() {
    size_t n = 5;
    char c = 'A';
    String str(n, c);
    assert(str.size() == n);
    assert(strcmp(str.data(), "AAAAA") == 0); 
}

void testDestructorReleasesMemory() {
    String* str = new String("Test");
    delete str;
}