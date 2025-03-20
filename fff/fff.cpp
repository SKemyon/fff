#include "fff.h"
String::StringData::StringData() : refCount(1), capacity(0), strLenght(0), data(nullptr) {}
String::StringData::StringData(const char* str) : refCount(1), capacity(strlen(str)), data(new char[strlen(str) + 1]) {
    strLenght = capacity;
    strcpy(data, str);
}
String::StringData::StringData(const char* str, size_t n) : refCount(1), strLenght(std::min(n, strlen(str))) {
    data = new char[strLenght + 1];
    capacity = strLenght;
    strncpy(data, str, strLenght);
    data[strLenght] = '\0';
}
String::StringData::StringData(size_t n, char c) : refCount(1), strLenght(n), capacity(n) {
    if (c == '\0') {
        strLenght = 0;
    }
    data = new char[n + 1];
    std::memset(data, c, n);
    data[n] = '\0';
}

String::StringData::~StringData() {
    delete[] data;
}

String::String() {
    stringData = new StringData();
}

String::String(const char* str) {
    if (str == nullptr) {
        stringData = new StringData();
        return;
    }
    stringData = new StringData(str);
}

String::String(const char* str, size_t n) {
    if (str == nullptr) {
        stringData = new StringData();
        return;
    }
    stringData = new StringData(str, n);
}


String::String(size_t n, char c) {

    stringData = new StringData(n, c);
}


String::String(const String& str) {
    stringData = str.stringData;

    (str.stringData->refCount)++;
}   
String::String(const String& str, size_t pos, size_t len) {

    if (!str.stringData->data)
    {
        stringData = new StringData();
    }
    size_t Length = str.stringData->strLenght;
    if (pos >= Length || pos < 0) {
        throw std::out_of_range("out of range in String(str, pos, len)");
    }
    if (len == npos || pos + len >= Length) {
        stringData = new StringData(str.stringData->data + pos);
    }
    stringData = new StringData(str.stringData->data + pos, len);

}


size_t String::size() const {
    return (stringData->strLenght);
}


size_t String::capacity() const {
    return (stringData->capacity);
}


void String::reserve(size_t n) {
    if (n == -1) {
        throw std::out_of_range("out of range in reserve()");
    }
    StringData* stringData_new = new StringData(n, '\0');
    if (stringData->data) {
        size_t copyCount = std::min(n, size());
        for (size_t i = 0; i < copyCount; ++i) {
            (stringData_new->data)[i] = (stringData->data)[i];
            stringData_new->strLenght = copyCount;
        }
    }

    if (--stringData->refCount == 0) {
        delete stringData;
    }
    stringData = stringData_new;
}


void String::clear() {
    if (size() != 0)
    {
        size_t n = stringData->capacity;
        if (--stringData->refCount == 0) {

            delete stringData;
        }
        stringData = new StringData(n, '\0');
    }
}


bool String::empty() const {
    return !(size() > 0);
}

size_t String::countRef() {
    return stringData->refCount;
}


char& String::at(size_t pos) {
    return operator[](pos);
}


const char& String::at(size_t pos) const {
    return operator[](pos);
}


const char& String::operator[](size_t pos) const {
    if (pos < size() && pos >= 0) {
        return stringData->data[pos];
    }
    throw std::out_of_range("String::at");
}


char& String::operator[](size_t pos) {

    if (pos < size() && pos >= 0) {
        if (stringData->refCount > 1) {
            stringData->refCount--;
            stringData = new StringData(stringData->data);
        }
        
        return stringData->data[pos];
    }
    throw std::out_of_range("out of range in operator[]");
}


char& String::back() {
    if (size() > 0) {
        if (stringData->refCount > 1) {
            stringData->refCount--;
            stringData = new StringData(stringData->data);
        }
        return stringData->data[size() - 1];
    }
    throw std::out_of_range("out of range in back()");
}


const char& String::back() const {
    if (size() > 0) {
        return stringData->data[size() - 1];
    }
    throw std::out_of_range("out of range in back()");
}


char& String::front() {
    if (size() > 0) {
        if (stringData->refCount > 1) {
            stringData->refCount--;
            stringData = new StringData(stringData->data);
        }
        return stringData->data[0];
    }
    throw std::out_of_range("out of range in front()");
}

const char& String::front() const {
    if (size() > 0) {
        return stringData->data[0];
    }
    throw std::out_of_range("out of range in front()");
}


String& String::operator+=(const String& str) {
    size_t len = str.size();
    if (len == 0)
    {
        return *this;
    }
    size_t this_len = size();
    size_t new_len = len + this_len;
    reserve(new_len);
    for (int i = 0; i <= len; i++) {
        stringData->data[this_len + i] = str.stringData->data[i];
    }
    stringData->strLenght = new_len;
    return *this;
}


String& String::operator+=(const char* str) {
    if (!str)
    {
        return *this;
    }
    size_t this_len = size();
    size_t len = strlen(str);
    size_t new_len = len + this_len;
    reserve(new_len);
    for (int i = 0; i <= len; i++) {
        (stringData->data)[this_len + i] = str[i];
    }
    stringData->strLenght = new_len;
    return *this;
}

String& String::operator+=(char c) {
    size_t this_len = size();
    size_t new_len = 1 + this_len;
    reserve(new_len);
    for (int i = 0; i < 1; i++) {
        (stringData->data)[this_len] = c;
    }
    (stringData->data)[this_len + 1] = '\0';
    stringData->strLenght = new_len;
    return *this;
}

String& String::operator=(const String& str) {
    if (this == &str) {
        return *this;
    }
    if (--stringData->refCount == 0) {

        delete stringData;
    }
    stringData = str.stringData;
    stringData->refCount++;
    return *this;
}


String& String::operator=(const char* str) {
    if (--stringData->refCount == 0) {
        delete stringData;
    }
    if (str == nullptr) {
        stringData = new StringData();
        return *this;
    }
    stringData = new StringData(str);
    return *this;
}


const char* String::data() const {
    if (empty()) {
        throw std::out_of_range("data is empty");
    }
    return stringData->data;
}

String& String::insert(size_t pos, const String& str) {
    if (str.empty()) {
        return *this;
    }
    insert(pos, str.data());
    return *this;
}

String& String::insert(size_t pos, const char* str) {
    size_t len = size();
    if (pos > len) {
        throw std::out_of_range("out of range in insert");
    }
    if (!stringData->data || pos == len) {
        *this += str;
        return *this;
    }
    String* box = new String(*this, pos, -1);
    reserve(pos);
    *this += str;
    *this += *box;
    delete box;
    return *this;
}

String& String::erase(size_t pos, size_t len) {
    size_t str_len = size();
    if (empty()) {
        return *this;
    }
    if (len == npos)
    {
        len = str_len - pos;
    }
    if (pos >= str_len || pos + len > str_len) {
        throw std::out_of_range("out of range in erase");
    }
    if (pos + len == str_len) {
        reserve(pos);
        return *this;
    }
    String box = String(*this, pos + len, -1);
    reserve(pos);
    *this += box;
  
    return *this;
}

String& String::replace(size_t pos, size_t len, const char* str) {
    erase(pos, len);
    insert(pos, str);
    return *this;
}

String& String::replace(size_t pos, size_t len, const String& str) {
    erase(pos, len);
    insert(pos, str);
    return *this;
}

String& String::replace(size_t pos, size_t len, size_t n, char c) {
    String Str = String(n, c);
    replace(pos, len, Str);
    return *this;
}

void String::swap(String& str) {
    String* box = new String(str);
    str.stringData = stringData;
    stringData = box->stringData;
    delete box;
}

size_t String::find(char c, size_t pos) const {
    if (pos >= size()) {
        npos;
    }
    for (size_t i = pos; i < size(); ++i) {
        if (data()[i] == c) {
            return i;
        }
    }
    return npos;
}


size_t String::find(const char* str, size_t pos) const {
    if (pos >= size()) {
        npos; 
    }
    std::string substring = str;
    size_t sub_len = substring.size();
    size_t data_len = size();
    std::string string = data();
    for (size_t i = pos; i <= data_len - sub_len; ++i) {
        if (string.compare(i, sub_len, substring) == 0) {
            return i; 
        }
    }
    return npos;
}



size_t String::find(const String& str, size_t pos) const {
    return find(str.data(), pos);
}

String* String::substr(size_t pos, size_t len) const{
    std::string str = data();
    std::string subStr = str.substr(pos, len);
    char* substring = new char[len + 1];
    subStr.copy(substring, len);
    substring[len] = '\0';
    String* return_String = new String(substring);
    delete[] substring;
    return return_String;
}

String::~String() {
    if (--stringData->refCount == 0) {

        delete stringData;
    }
}