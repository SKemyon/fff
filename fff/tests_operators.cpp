#include <cassert>
#include <iostream>
#include <stdexcept>
#include "fff.h"
#include "tests_operator.h"
#include <gtest/gtest.h>

TEST(StringTest, OperatorSquareBrackets) {
    String str("Hello");
    EXPECT_EQ(str[0], 'H');
    EXPECT_EQ(str[4], 'o');

    EXPECT_THROW(str[5], std::out_of_range);
    EXPECT_THROW(str[-1], std::out_of_range);
}

TEST(StringTest, Back) {
    String str("Hello");
    EXPECT_EQ(str.back(), 'o');

    String emptyStr;
    EXPECT_THROW(emptyStr.back(), std::out_of_range);
}

TEST(StringTest, Front) {
    String str("Hello");
    EXPECT_EQ(str.front(), 'H');

    String emptyStr;
    EXPECT_THROW(emptyStr.front(), std::out_of_range);
}

TEST(StringTest, OperatorPlusEqualString) {
    String str1("Hello");
    String str2(" World");

    str1 += str2;
    EXPECT_STREQ(str1.data(), "Hello World");

    String original = str1;
    str1 += String("");
    EXPECT_STREQ(str1.data(), original.data());
}

TEST(StringTest, OperatorPlusEqualCStr) {
    String str("Hello");
    str += " World";
    EXPECT_STREQ(str.data(), "Hello World");

    String original = str;
    str += nullptr;
    EXPECT_STREQ(str.data(), original.data());
}

TEST(StringTest, OperatorPlusEqualChar) {
    String str("Hello");
    str += '!';
    EXPECT_STREQ(str.data(), "Hello!");

    String emptyStr;
    emptyStr += 'A';
    EXPECT_STREQ(emptyStr.data(), "A");
}

TEST(StringTest, AssignmentOperator) {
    String str1("Hello");
    String str2 = str1;

    EXPECT_STREQ(str1.data(), str2.data());

    str1 = str1;
    EXPECT_STREQ(str1.data(), "Hello");

    String emptyStr;
    emptyStr = nullptr; 
    EXPECT_TRUE(emptyStr.empty());
}

TEST(StringTest, Data) {
    String str("Hello");
    EXPECT_STREQ(str.data(), "Hello");

    String emptyStr;
    EXPECT_THROW(emptyStr.data(), std::out_of_range);
}






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