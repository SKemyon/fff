#include "String.h"  
#include <gtest/gtest.h>

TEST(StringTest, DefaultConstructor) {
    String str;
    EXPECT_EQ(str.size(), 0);
    EXPECT_THROW(str.data(), std::out_of_range);
}

TEST(StringTest, ConstructorFromCStr) {
    const char* testStr = "Hello, World!";
    String str(testStr);
    EXPECT_EQ(str.size(), strlen(testStr));
    EXPECT_STREQ(str.data(), testStr);
}

TEST(StringTest, ConstructorFromNullCStr) {
    String str(nullptr);
    EXPECT_EQ(str.size(), 0);
    EXPECT_THROW(str.data(), std::out_of_range);
}

TEST(StringTest, ConstructorFromCStrWithLength) {
    const char* testStr = "Hello, World!";
    String str(testStr, 5);
    EXPECT_EQ(str.size(), 5);
    EXPECT_STREQ(str.data(), "Hello");
}

TEST(StringTest, ConstructorFromLengthAndChar) {
    size_t n = 5;
    char c = 'A';
    String str(n, c);
    EXPECT_EQ(str.size(), n);
    EXPECT_STREQ(str.data(), "AAAAA");
}

TEST(StringTest, DestructorReleasesMemory) {
    String* str = new String("Test");
    delete str;// утечки
}

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

TEST(StringTest, countRef) {
    String str1("");
    String str3 = str1;
    String str2(nullptr);
    String str4 = str2;
    str1 = str4;
    EXPECT_EQ(str1.countRef(), 3);
    EXPECT_EQ(str3.countRef(), 1);
    
    EXPECT_EQ(str2.countRef(), 3);
    EXPECT_EQ(str4.countRef(), 3);// в
}

TEST(StringTest, Capacity) {////////
    String str1(nullptr);
    String str2("fffffffffff");
    EXPECT_EQ(str1.capacity(), 0);
    EXPECT_EQ(str2.capacity(), 11);
    str1.reserve(7);
    str2.reserve(3);
    EXPECT_EQ(str1.capacity(), 7);
    EXPECT_EQ(str2.capacity(), 3);
    String str3;
    EXPECT_EQ(str3.capacity(), 0);
    String str4("");
    EXPECT_EQ(str4.capacity(), 0);
    str4 += "AAAA";
    EXPECT_EQ(str4.capacity(), 4);
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

//TEST(StringTest, ReplaceCStr) {
//    String str("Hello, World!");
//    str.replace(7, 5, "Universe");
//    EXPECT_STREQ(str.data(), "Hello, Universe!");
//
//    str.replace(6, 8, "");
//    EXPECT_STREQ(str.data(), "Hello,e!");
//
//    EXPECT_THROW(str.replace(15, 1, "Test"), std::out_of_range);
//
//    EXPECT_NO_THROW(str.replace(0, String::npos, "Start"));
//    EXPECT_STREQ(str.data(), "Start");
//}

TEST(StringTest, ReplaceCStr) {
    String str("Hello, World!");

    str.replace(7, 5, "Universe");
    EXPECT_STREQ(str.data(), "Hello, Universe!");

    str.replace(6, 8, "");
    EXPECT_STREQ(str.data(), "Hello,e!");

    EXPECT_THROW(str.replace(15, 1, "Test"), std::out_of_range);

    EXPECT_NO_THROW(str.replace(0, String::npos, "Start"));
    EXPECT_STREQ(str.data(), "Start");

    str.replace(0, 0, "Hello");
    EXPECT_STREQ(str.data(), "HelloStart");

    str.replace(5, 5, "!!!");
    EXPECT_STREQ(str.data(), "Hello!!!");


    str.replace(5, 3, "Wonderful");
    EXPECT_STREQ(str.data(), "HelloWonderful");

    str.replace(0, str.size(), "New String");
    EXPECT_STREQ(str.data(), "New String");

    str.replace(0, 4, "Test ");
    EXPECT_STREQ(str.data(), "Test String");

    String emptyStr("");
    EXPECT_NO_THROW(emptyStr.replace(0, 0, "Empty"));
    EXPECT_STREQ(emptyStr.data(), "Empty");


    String singleCharStr("A");
    EXPECT_NO_THROW(singleCharStr.replace(0, 1, "B"));
    EXPECT_STREQ(singleCharStr.data(), "B");

    EXPECT_THROW(str.replace(-1, 1, "Test"), std::out_of_range);
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

    String subStr = str.substr(0);
    std::cout << subStr.data();
    EXPECT_STREQ(subStr.data(), "Hello, World!");
    EXPECT_THROW(str.substr(15, 5), std::out_of_range);
}

TEST(StringTest, InsertNullptrSTR) {
    String str(nullptr);
    String inv("dddd");
    EXPECT_THROW(str.insert(6, inv), std::out_of_range);
}

TEST(StringTest, InsertAtBeginningSTR) {
    String str("Hello");
    String str1("Start ");
    str.insert(0, str1);
    EXPECT_STREQ(str.data(), "Start Hello");
}

TEST(StringTest, InsertAtEndSTR) {
    String str("Hello");
    String str1(" World");
    str.insert(5, str1);
    EXPECT_STREQ(str.data(), "Hello World");
}

TEST(StringTest, InsertOutOfRangeSTR) {
    String str("Hello");
    String str1("Out of range");
    EXPECT_THROW(str.insert(10, str1), std::out_of_range);
}

TEST(StringTest, InsertAtPositionEqualToLengthSTR) {
    String str("Hello");
    String str1("!");
    str.insert(5, str1);
    EXPECT_STREQ(str.data(), "Hello!");
}

TEST(StringTest, InsertNullptr) {
    String str(nullptr);
    EXPECT_THROW(str.insert(6, "Invalid"), std::out_of_range);
}

TEST(StringTest, InsertAtBeginning) {
    String str("Hello");
    str.insert(0, "Start ");
    EXPECT_STREQ(str.data(), "Start Hello");
}

TEST(StringTest, InsertAtEnd) {
    String str("Hello");
    str.insert(5, " World");
    EXPECT_STREQ(str.data(), "Hello World");
}

TEST(StringTest, InsertInMiddle) {
    String str("Hello");
    str.insert(3, "lo");

    EXPECT_STREQ(str.data(), "Hellolo");
}

TEST(StringTest, InsertOutOfRange) {
    String str("Hello");
    EXPECT_THROW(str.insert(10, "Out of range"), std::out_of_range);
}

TEST(StringTest, InsertEmptyString) {
    String str("Hello");
    str.insert(2, "");
    EXPECT_STREQ(str.data(), "Hello");
}

TEST(StringTest, InsertAtPositionEqualToLength) {
    String str("Hello");
    str.insert(5, "!");
    EXPECT_STREQ(str.data(), "Hello!");
}

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
