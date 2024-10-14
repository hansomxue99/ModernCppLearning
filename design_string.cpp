#include <iostream>
#include <cstddef>

struct MyString {
    MyString(char *s="") {
        std::cout << "default construct\n";
        m_length = std::strlen(s);
        m_data = new char[m_length + 1];    // 包含\0
        std::strcpy(m_data, s);
    }

    ~MyString() {
        std::cout << "deconstruct\n";
        delete[] m_data;
    }

    MyString(const MyString& other) {
        std::cout << "copy construct\n";
        m_length = other.m_length;
        m_data = new char[m_length + 1];
        std::strcpy(m_data, other.m_data);
    }

    MyString& operator=(const MyString& other) {
        std::cout << "copy assign\n";
        if (this != &other) {
            delete[] m_data;
            m_length = other.m_length;
            m_data = new char[m_length + 1];
            std::strcpy(m_data, other.m_data);
        }

        return *this;
    }

    MyString(MyString&& other) : m_data(nullptr), m_length(0) {
        std::cout << "move construct\n";
        other.swap(*this);
    }

    MyString& operator=(MyString&& other) {
        std::cout << "move assign\n";
        if (this != &other) {
            // MyString(std::move(other)).swap(*this);
            delete[] m_data;
            m_length = other.m_length;
            m_data = other.m_data;
            other.m_data = nullptr;
            other.m_length = 0;
        }
        return *this;
    }

    friend std::ostream& operator<<(std::ostream& os, const MyString& str) {
        os << str.m_data;
        return os;
    }

private:
    void swap(MyString& other) {
        std::swap(m_data, other.m_data);
        std::swap(m_length, other.m_length);
    }

    char *m_data;
    size_t m_length;
};

int main() {
    MyString str1("Hello, world!");
    MyString str2 = str1;  // 测试拷贝构造
    MyString str3;
    str3 = std::move(str1);  // 测试移动赋值

    std::cout << "str2: " << str2 << std::endl;
    std::cout << "str3: " << str3 << std::endl;

    return 0;
}