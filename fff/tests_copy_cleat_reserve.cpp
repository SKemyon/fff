#include <cassert>
#include <iostream>
#include <stdexcept>
#include "fff.h"
#include "tests_copy_cleat_reserve.h"
#include <gtest/gtest.h>

TEST(StringTest, CopyConstructor) {
    String str1("Hello, World!");
    String str2(str1);

    EXPECT_EQ(str2.size(), str1.size());
    EXPECT_STREQ(str2.data(), str1.data());
    EXPECT_EQ(str1.countRef(), 2);
}

TEST(StringTest, CopyConstructorWithEmptyString) {
    String str1;
    String str2(str1);

    EXPECT_EQ(str2.size(), 0);
    EXPECT_EQ(str1.countRef(), 2);
}

TEST(StringTest, SubstringConstructor) {
    String str1("Hello, World!");
    String str2(str1, 7, 5);

    EXPECT_EQ(str2.size(), 5);
    EXPECT_STREQ(str2.data(), "World");
}

TEST(StringTest, SubstringConstructorOutOfRange) {
    String str1("Hello, World!");

    EXPECT_THROW({
        try {
            String str2(str1, 15, 5);
            FAIL() << "Expected std::out_of_range";
        }
 catch (const std::out_of_range&) {
     
     throw;
 }
        }, std::out_of_range);
}

TEST(StringTest, SubstringConstructorWithNpos) {
    String str1("Hello, World!");
    String str2(str1, 7, String::npos);

    EXPECT_EQ(str2.size(), 6);
    EXPECT_STREQ(str2.data(), "World!");
}

TEST(StringTest, Size) {
    String str1("Hello");

    EXPECT_EQ(str1.size(), 5);

    String str2;
    EXPECT_EQ(str2.size(), 0);
}

TEST(StringTest, Capacity) {
    String str1("Hello");

    EXPECT_GE(str1.capacity(), str1.size());
}

TEST(StringTest, Reserve) {
    String str1("Hello");
    size_t oldCapacity = str1.capacity();

    str1.reserve(20);

    EXPECT_GE(str1.capacity(), 20);
    EXPECT_EQ(str1.size(), 5);
}

TEST(StringTest, ReserveOutOfRange) {
    String str1("Hello");

    EXPECT_THROW({
        try {
            str1.reserve(static_cast<size_t>(-1));
            FAIL() << "Expected std::out_of_range";
        }
 catch (const std::out_of_range&) {
     throw;
 }
        }, std::out_of_range);
}

TEST(StringTest, Clear) {
    String str1("Hello");
    str1.clear();

    EXPECT_EQ(str1.size(), 0);
    EXPECT_GT(str1.capacity(), 0);
}

TEST(StringTest, ClearOnEmptyString) {
    String str1;
    str1.clear();

    EXPECT_EQ(str1.size(), 0);
}





void AllCopyClearReserv() {
    testCopyConstructor();

    testCopyConstructorWithEmptyString();

    testSubstringConstructor();

    testSubstringConstructorOutOfRange();

    testSubstringConstructorWithNpos();

    testSize();

    testCapacity();

    testReserve();

    testReserveOutOfRange();

    testClear();

    testClearOnEmptyString();
}

void testCopyConstructor() {
    String str1("Hello, World!");
    String str2(str1);
    assert(str2.size() == str1.size());
    assert(strcmp(str2.data(), str1.data()) == 0);
    assert(str1.countRef() == 2);
}

void testCopyConstructorWithEmptyString() {
    String str1;
    String str2(str1);
    assert(str2.size() == 0);
    assert(str1.countRef() == 2); 
}

void testSubstringConstructor() {
    String str1("Hello, World!");
    String str2(str1, 7, 5); 
    assert(str2.size() == 5);
    assert(strcmp(str2.data(), "World") == 0);
}

void testSubstringConstructorOutOfRange() {
    String str1("Hello, World!");

    try {
        String str2(str1, 15, 5);
        assert(false); 
    }
    catch (const std::out_of_range&) {
    }
}

void testSubstringConstructorWithNpos() {
    String str1("Hello, World!");
    String str2(str1, 7, String::npos); 

    assert(str2.size() == 6);
    assert(strcmp(str2.data(), "World!") == 0);
}

void testSize() {
    String str1("Hello");
    assert(str1.size() == 5);

    String str2;
    assert(str2.size() == 0);
}

void testCapacity() {
    String str1("Hello");
    assert(str1.capacity() >= str1.size());
}

void testReserve() {
    String str1("Hello");
    size_t oldCapacity = str1.capacity();

    str1.reserve(20);

    assert(str1.capacity() >= 20);
    assert(str1.size() == 5); 
}

void testReserveOutOfRange() {
    String str1("Hello");

    try {
        str1.reserve(-1);
        assert(false);
    }
    catch (const std::out_of_range&) {
        
    }
}

void testClear() {
    String str1("Hello");
    str1.clear();

    assert(str1.size() == 0);
    assert(str1.capacity() > 0);
}

void testClearOnEmptyString() {
    String str1;
    str1.clear(); 

    assert(str1.size() == 0);
}