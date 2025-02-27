#define _CRT_SECURE_NO_WARNINGS
#include <iostream>

class String {
private:
    struct StringData {
        char* data;
        int refCount;
        size_t capacity;

        StringData(const char* str) : refCount(1) {
            data = new char[strlen(str) + 1];
            capacity += strlen(str);
            std::cout << "malloc string data" << std::endl;
            strcpy(data, str);
        }

        StringData(const char* str, size_t n) : refCount(1) {
            int len = strlen(str);
            if (n >= len) {
                data = new char[len+1];
                capacity += len;
                strcpy(data, str);
            }
            else if (n >= 0 && n < len) {
                data = new char[n + 1];
                capacity += n;
                strncpy(data, str, n);
                data[n] = '\0';
            }
            std::cout << "malloc string data" << std::endl;

      
        }

        StringData(size_t n, char c) : refCount(1) {
            data = new char[n + 1];
            capacity += n;
            std::cout << "malloc string data" << std::endl;
            for (int i = 0; i < n; i++) {
                data[i] = c;
            }
            data[n] = '\0';
        }


        ~StringData() {
            std::cout << "free string data" << std::endl;
            delete[] data;
        }
    };


public:
    StringData* stringData;

    static const size_t npos = -1;

    String() : stringData(nullptr) {}
    String(const char* str) {
        stringData = new StringData(str);
        std::cout << "malloc string" << std::endl;
    }
    String(const char* str, size_t n) {

        stringData = new StringData(str, n);
        std::cout << "malloc string" << std::endl;
    }

    String(size_t n, char c) {

        stringData = new StringData(n, c);
        std::cout << "malloc string" << std::endl;
    }

    String(const String& str) {
        stringData = str.stringData;
        (str.stringData->refCount)++;
    }

    String(const String& str, size_t pos, size_t len = npos) {
        if (!str.stringData) {
            stringData = nullptr;
            return;
        }

        size_t strLength = strlen(str.stringData->data);

        // Проверяем корректность позиции
        if (pos > strLength) {
            throw std::out_of_range("Position out of range");
        }

        // Устанавливаем длину подстроки
        if (len == npos || pos + len > strLength) {
            len = strLength - pos; // Если len больше оставшейся части строки, берем остаток
        }

        
        stringData = new StringData(str.stringData->data + pos); //нужен другой конструктор
        stringData->data[len] = '\0'; 
        std::cout << "malloc substring" << std::endl;

    }

    size_t size() const {
        return strlen(stringData->data);
    }

    size_t capacity() const {
        return stringData->capacity;
    }

    void reserve(size_t n = 0) {
        if (n < 0) {
            return;
        }
        StringData* stringData_new = new StringData(n + 1, '\0');
        if (stringData->data) {
            size_t copyCount = std::min(n, size());
            for (size_t i = 0; i < copyCount; ++i) {
                (stringData_new->data)[i] = (stringData->data)[i];
            }
        }
        if (--stringData->refCount == 0) {
            delete stringData;
        }
        stringData = stringData_new;
        stringData->capacity = n;
    }

    void clear() {
        if (stringData->data)
        {
            int i = 0;
            while (stringData->data[i] != '\0')
            {
                stringData->data[i] = '\0';
                i++;
            }
        }
    }

    bool empty() {
        if (stringData->data && size() > 0)
        {
            return false;
        }
        return true;
    }

    char& at(size_t pos) {
        if (pos < size() && pos >= 0) {
            char& at = stringData->data[pos];
            return at;
        }
    }

    const char& at(size_t pos) const {
        if (pos < size() && pos >= 0) {
            char& at = stringData->data[pos];
            return at;
        }
    }

    const char& operator[](size_t pos) const{
        if (pos < size() && pos >= 0) {
            char& at = stringData->data[pos];
            return at;
        }
        else {
            throw std::out_of_range("Position out of range");
        }
        
    }

    char& operator[](size_t pos){
        if (pos < size() && pos >= 0) {
            char& at = stringData->data[pos];
            return at;
        }
        else {
            throw std::out_of_range("Position out of range");
        }

    }

    char& back() {
        if (stringData->data && size() > 0) {//отрицательные числа ?
            return stringData->data[size()-1];
        }
    }

    const char& back() const{
        if (stringData->data && size() > 0) {//отрицательные числа ?
            return stringData->data[size() - 1];
        }
    }

    char& front() {
        if (stringData->data && size() > 0) {//отрицательные числа ?
            return stringData->data[0];
        }
    }

    const char& front() const{
        if (stringData->data && size() > 0) {//отрицательные числа ?
            return stringData->data[0];
        }
    }

    String& operator+=(const String& str) {
        size_t this_len = size();
        size_t len = str.size();
        size_t new_len = len + this_len;
        if (stringData->data && str.stringData->data) {
            if (capacity() < new_len) {
                reserve(new_len);
            }         
            for (int i = 0; i <= len; i++) {
                stringData->data[this_len + i] = str.stringData->data[i];
            }
            return *this;
        }    
    }


    ~String() {
        std::cout << "free string" << std::endl;
        if (--stringData->refCount == 0) {
            
            delete stringData;
        }
    }
};

int main()
{
    String *a = new String("gfggffg", 8);
    String *c = new String(*a, 0, 8);
    (*a).front() = 'n';
    c->reserve(9);
    *a += *c;
    char* b = a->stringData->data;
    
    int ref = a->stringData->refCount;
    
    //c->clear();
    std::cout << b[0] << std::endl;
    std::cout << a->stringData->data << std::endl;
    std::cout << "Entered integer is: " << ref << std::endl;
    std::cout << "size: " << c->size() << std::endl;
    std::cout << "capaciy: " << c->capacity() << std::endl;
    delete c;
    delete a;
}