#include <cassert>
#include <iostream>
#include <stdexcept>
#include "fff.h"
#include "tests_operator.h"
void AllOperators() {
    testOperatorSquareBrackets();
    testBack();
    testFront();
    testOperatorPlusEqualString();
    testOperatorPlusEqualCStr();
    testOperatorPlusEqualChar();
    testAssignmentOperator();
    testData();
}

void testOperatorSquareBrackets() {
    String str("Hello");
    assert(str[0] == 'H');
    assert(str[4] == 'o');
    try {
        char c = str[5];
        assert(false);
    }
    catch (const std::out_of_range&) {}

    try {
        char c = str[-1];
        assert(false);
    }
    catch (const std::out_of_range&) {}
}


void testBack() {
    String str("Hello");
    assert(str.back() == 'o');
    String emptyStr;
    try {
        char c = emptyStr.back(); 
        assert(false);
    }
    catch (const std::out_of_range&) {}
}



void testFront() {
    String str("Hello");
    assert(str.front() == 'H');
    String emptyStr;
    try {
        char c = emptyStr.front();
        assert(false);
    }
    catch (const std::out_of_range&) {}
}



void testOperatorPlusEqualString() {
    String str1("Hello");
    String str2(" World");

    str1 += str2;
    assert(strcmp(str1.data(), "Hello World") == 0);
    String original = str1;
    str1 += String("");
    assert(strcmp(str1.data(), original.data()) == 0);
}


void testOperatorPlusEqualCStr() {
    String str("Hello");
    str += " World";
    assert(strcmp(str.data(), "Hello World") == 0);
    String original = str;
    str += nullptr;
    assert(strcmp(str.data(), original.data()) == 0);
}



void testOperatorPlusEqualChar() {
    String str("Hello");
    str += '!';
    assert(strcmp(str.data(), "Hello!") == 0);
    String emptyStr;
    emptyStr += 'A';
    assert(strcmp(emptyStr.data(), "A") == 0);
}


void testAssignmentOperator() {
    String str1("Hello");
    String str2 = str1;
    assert(strcmp(str1.data(), str2.data()) == 0);
    assert(str1.data() == str2.data());
    str1 = str1;
    assert(strcmp(str1.data(), "Hello") == 0);
    String emptyStr;
    emptyStr = nullptr;
    assert(emptyStr.empty());
}



void testData() {
    String str("Hello");
    assert(strcmp(str.data(), "Hello") == 0);
    String emptyStr;
    try {
        const char* data = emptyStr.data();
        assert(false);
    }
    catch (const std::out_of_range&) {}
}