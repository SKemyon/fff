#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
class String {

    class StringData {
    private:
        char* data;
        int refCount;
        size_t capacity;
        size_t strLenght;
    public:
        StringData();
        StringData(const char* str);
        StringData(const char* str, size_t n);
        StringData(size_t n, char c);
        ~StringData();
        friend class String;
    };
private:
    StringData* stringData{};
    
public:
    static const size_t npos = -1;
    String();
    String(const char* str);
    String(const char* str, size_t n);
    String(size_t n, char c);
    String(const String& str);
    String(const String& str, size_t pos, size_t len = npos);
    size_t size() const;
    size_t capacity() const;
    void reserve(size_t n = 0);
    void clear();
    bool empty() const;
    size_t countRef() const;
    char& at(size_t pos);
    const char& at(size_t pos) const;
    const char& operator[](size_t pos) const;
    char& operator[](size_t pos);
    char& back();
    const char& back() const;
    char& front();
    const char& front() const;
    String& operator+=(const String& str);
    String& operator+=(const char* str);
    String& operator+=(char c);
    String& operator=(const String& str);
    String& operator=(const char* str);
    const char* data() const;
    String& insert(size_t pos, const String& str);
    String& insert(size_t pos, const char* str);
    String& erase(size_t pos = 0, size_t len = npos);
    String& replace(size_t pos, size_t len, const char* str);
    String& replace(size_t pos, size_t len, const String& str);
    String& replace(size_t pos, size_t len, size_t n, char c);
    void swap(String& str);
    size_t find(const String& str, size_t pos = 0) const;
    size_t find(const char* str, size_t pos = 0) const;
    size_t find(char c, size_t pos = 0) const;
    String substr (size_t pos = 0, size_t len = npos) const;
    int compare(const String& str);
    ~String();
};