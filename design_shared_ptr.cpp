#include <iostream>
#include <cstddef>
using namespace std;

template <typename T>
struct MySharedPtr {
    MySharedPtr(T* ptr = nullptr) : m_ptr(ptr), m_cnt(ptr ? new size_t(1) : nullptr) {}
    ~MySharedPtr() { release(); }

    MySharedPtr(const MySharedPtr& other) : m_ptr(other.m_ptr), m_cnt(other.m_cnt) {
        if (m_cnt) {
            ++(*m_cnt);
        }
        cout << "copy constuct\n";
    }

    MySharedPtr& operator=(const MySharedPtr& other) {
        cout << "copy assign\n";
        if (m_ptr == other.m_ptr) {
            return *this;
        }
        release();
        m_ptr = other.m_ptr;
        m_cnt = other.m_cnt;
        if (m_cnt) ++(*m_cnt);
        return *this;
    }

    MySharedPtr(MySharedPtr&& other) : m_ptr(nullptr), m_cnt(nullptr) {
        other.swap(*this);
    }

    MySharedPtr& operator=(MySharedPtr&& other) {
        MySharedPtr(std::move(other)).swap(*this);
        return *this;
    }

    size_t use_count() const { return *m_cnt; }
    T& operator*() const { return *m_ptr; }
    T* operator->() const { return m_ptr; }

private:
    void release() {
        if (m_cnt && --(*m_cnt) == 0) {
            delete m_ptr;
            delete m_cnt;
            m_ptr = nullptr;
            m_cnt = nullptr;
        }
    }

    void swap(MySharedPtr& other) {
        std::swap(m_ptr, other.m_ptr);
        std::swap(m_cnt, other.m_cnt);
    }

    T* m_ptr;
    size_t* m_cnt;
};

class Demo {
public:
    Demo() { std::cout << "Demo construct\n"; }
    ~Demo() { std::cout << "Demo deconstruct\n"; }
    void do_something() { std::cout << "Demo::do_something\n"; }
};

int main () {
    MySharedPtr<Demo> p1(new Demo);
    MySharedPtr<Demo> p4(new Demo);
    {
        MySharedPtr<Demo> p2 = p1;
        MySharedPtr<Demo> p3;
        p3 = std::move(p4);
        cout << "p1 use count: " << p1.use_count() << '\n';
        cout << "p3 use count: " << p3.use_count() << '\n';
    }
    std::cout << "p1 use count: " << p1.use_count() << '\n';

    return 0;
}