#include "fff.h"
#include "tests_insert.h"
void runAllInsert() {
    testInsertAtBeginning();
    testInsertAtEnd();
    testInsertInMiddle();
    testInsertOutOfRange();
    testInsertEmptyString();
    testInsertAtPositionEqualToLength();
    testInsertNulptr();
    testInsertNulptrSTR();
    testInsertAtBeginningSTR();
    testInsertAtEndSTR();
    testInsertOutOfRangeSTR();
    testInsertAtPositionEqualToLengthSTR();
}

void testInsertNulptrSTR() {
    String str(nullptr);
    String inv("dddd");
    try {
        str.insert(6, inv);
        assert(false);
    }
    catch (const std::out_of_range&) {
    }
}
void testInsertAtBeginningSTR() {
    String str("Hello");
    String str1("Start ");
    str.insert(0, str1);
    assert(strcmp(str.data(), "Start Hello") == 0);
}

void testInsertAtEndSTR() {
    String str("Hello");
    String str1(" World");
    str.insert(5, str1);
    assert(strcmp(str.data(), "Hello World") == 0);
}

void testInsertOutOfRangeSTR() {
    String str("Hello");
    String str1("Out of range");
    try {
        str.insert(10, str1);
        assert(false);
    }
    catch (const std::out_of_range&) {
    }
}

void testInsertAtPositionEqualToLengthSTR() {
    String str("Hello");
    String str1("!");
    str.insert(5, str1);
    assert(strcmp(str.data(), "Hello!") == 0);
}
///////////////////////////////////////////////////////////////////////////////////////////////
void testInsertNulptr() {
    String str(nullptr);
    try {
        str.insert(6, "Invalid");
        assert(false);
    }
    catch (const std::out_of_range&) {
    }
}
void testInsertAtBeginning() {
    String str("Hello");
    str.insert(0, "Start ");
    assert(strcmp(str.data(), "Start Hello") == 0);
}

void testInsertAtEnd() {
    String str("Hello");
    str.insert(5, " World");
    assert(strcmp(str.data(), "Hello World") == 0);
}

void testInsertInMiddle() {
    String str("Hello");
    str.insert(3, "lo");
    
    assert(strcmp(str.data(),"Hellolo") == 0);
}

void testInsertOutOfRange() {
    String str("Hello");
    try {
        str.insert(10, "Out of range");
        assert(false);
    }
    catch (const std::out_of_range&) {
    }
}

void testInsertEmptyString() {
    String str("Hello");
    str.insert(2, "");
    assert(strcmp(str.data(), "Hello") == 0);
}

void testInsertAtPositionEqualToLength() {
    String str("Hello");
    str.insert(5, "!");
    assert(strcmp(str.data(), "Hello!") == 0);
}


