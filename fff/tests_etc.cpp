#include <cassert>
#include <iostream>
#include <stdexcept>
#include "fff.h"
#include "tests_etc.h"
#include <gtest/gtest.h>

TEST(StringTest, Erase) {
    String str("Hello, World!");
    str.erase(5, 7);
    EXPECT_STREQ(str.data(), "Hello!");

    str.erase(5);
    EXPECT_STREQ(str.data(), "Hello");

    String emptyStr;
    emptyStr.erase(0, 1);
    EXPECT_TRUE(emptyStr.empty());

    EXPECT_THROW(str.erase(10, 1), std::out_of_range);

    EXPECT_NO_THROW(str.erase(0, String::npos));
    EXPECT_TRUE(str.empty());
}

TEST(StringTest, ReplaceCStr) {
    String str("Hello, World!");
    str.replace(7, 5, "Universe");
    EXPECT_STREQ(str.data(), "Hello, Universe!");

    str.replace(6, 8, "");
    EXPECT_STREQ(str.data(), "Hello,e!");

    EXPECT_THROW(str.replace(15, 1, "Test"), std::out_of_range);

    EXPECT_NO_THROW(str.replace(0, String::npos, "Start"));
    EXPECT_STREQ(str.data(), "Start");
}

TEST(StringTest, ReplaceString) {
    String str("Hello, World!");
    String replacement("Earth");

    str.replace(7, 5, replacement);
    EXPECT_STREQ(str.data(), "Hello, Earth!");

    str.replace(6, 6, String(""));
    EXPECT_STREQ(str.data(), "Hello,!");

    EXPECT_THROW(str.replace(15, 1, replacement), std::out_of_range);
}

TEST(StringTest, ReplaceChar) {
    String str("Hello, World!");

    str.replace(7, 5, 3, '*');
    EXPECT_STREQ(str.data(), "Hello, ***!");

    EXPECT_THROW(str.replace(15, 1, 1, '*'), std::out_of_range);
}

TEST(StringTest, Swap) {
    String str1("Hello");
    String str2("World");

    str1.swap(str2);
    EXPECT_STREQ(str1.data(), "World");
    EXPECT_STREQ(str2.data(), "Hello");

    str1.swap(str1); //самосмена
    EXPECT_STREQ(str1.data(), "World");
}

TEST(StringTest, FindString) {
    String str("Hello, World!");

    size_t pos = str.find(String("World"));
    EXPECT_EQ(pos, 7);

    pos = str.find(String("NotFound"));
    EXPECT_EQ(pos, String::npos);
}

TEST(StringTest, FindCStr) {
    String str("Hello, World!");

    size_t pos = str.find("World");
    EXPECT_EQ(pos, 7);

    pos = str.find("NotFound");
    EXPECT_EQ(pos, String::npos);
}

TEST(StringTest, FindChar) {
    String str("Hello, World!");

    size_t pos = str.find('W');
    EXPECT_EQ(pos, 7);

    pos = str.find('Z');
    EXPECT_EQ(pos, String::npos);
}

TEST(StringTest, Substr) {
    String str("Hello, World!");

    String* subStr = str.substr(7, 5);
    EXPECT_STREQ(subStr->data(), "World");

    EXPECT_THROW(str.substr(15, 5), std::out_of_range);

    delete subStr; 
}



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