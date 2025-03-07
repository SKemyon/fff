#include <cassert>
#include <iostream>
#include <stdexcept>
#include "fff.h"
#include "tests_etc.h"

void runAllTests() {
    testErase();
    testReplaceCStr();
    testReplaceString();
    testReplaceChar();
    testSwap();
    testFindString();
    testFindCStr();
    testFindChar();
    testSubstr();
}

void testErase() {
    String str("Hello, World!");
    str.erase(5, 7);
    assert(strcmp(str.data(), "Hello!")==0);
    str.erase(5);
    assert(strcmp(str.data(), "Hello") == 0);
    String emptyStr;
    emptyStr.erase(0, 1);
    assert(emptyStr.empty());
    try {
        str.erase(10, 1); 
        assert(false);
    }
    catch (const std::out_of_range&) {}

    try {
        str.erase(0, String::npos);
        assert(str.empty());
    }
    catch (...) {
        assert(false);
    }
}

void testReplaceCStr() {
    String str("Hello, World!");
    str.replace(7, 5, "Universe");
    assert(strcmp(str.data(), "Hello, Universe!") == 0);
    str.replace(6, 8, "");
    assert(strcmp(str.data(), "Hello,e!") == 0);
    try {
        str.replace(15, 1, "Test");
        assert(false);
    }
    catch (const std::out_of_range&) {}

    try {
        str.replace(0, String::npos, "Start");
        assert(strcmp(str.data(), "Start") == 0);
    }
    catch (...) {
        assert(false); 
    }
}

void testReplaceString() {
    String str("Hello, World!");
    String replacement("Earth");
    str.replace(7, 5, replacement);
    assert(strcmp(str.data(), "Hello, Earth!") == 0);
    str.replace(6, 6, String(""));
    assert(strcmp(str.data(), "Hello,!") == 0);
    try {
        str.replace(15, 1, replacement);
        assert(false);
    }
    catch (const std::out_of_range&) {}
}

void testReplaceChar() {
    String str("Hello, World!");
    str.replace(7, 5, 3, '*');
    assert(strcmp(str.data(), "Hello, ***!") == 0);
    assert(strcmp(str.data(), "Hello, ***!") == 0);
    try {
        str.replace(15, 1, 1, '*');
        assert(false);
    }
    catch (const std::out_of_range&) {}
}

void testSwap() {
    String str1("Hello");
    String str2("World");
    str1.swap(str2);
    assert(strcmp(str1.data(), "World") == 0);
    assert(strcmp(str2.data(), "Hello") == 0);
    str1.swap(str1);
    assert(strcmp(str1.data(), "World") == 0);
}

void testFindString() {
    String str("Hello, World!");

    size_t pos = str.find(String("World"));

    assert(pos == 7);

    pos = str.find(String("NotFound"));

    assert(pos == String::npos); 
}

void testFindCStr() {
    String str("Hello, World!");

    size_t pos = str.find("World");

    assert(pos == 7);

    pos = str.find("NotFound");
assert(pos == String::npos);
}

void testFindChar() {
    String str("Hello, World!");

    size_t pos = str.find('W');

    assert(pos == 7); 

    pos = str.find('Z');

    assert(pos == String::npos);
}


void testSubstr() {
    String str("Hello, World!");

    String* subStr = str.substr(7, 5);

    assert(strcmp((*subStr).data(), "World") == 0);

    
    try {
        subStr = str.substr(15, 5);
        assert(false);
    }
    catch (const std::out_of_range&) {}

    delete subStr;
}