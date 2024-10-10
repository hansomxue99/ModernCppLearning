#include <iostream>
#include <cstddef>
#include <cassert>

template <typename T>
struct CircleQueue {
    explicit CircleQueue(size_t n) : m_capacity(n), m_front(0), m_rear(0), m_size(0), m_data(new T[n]) {}
    ~CircleQueue() { delete[] m_data; }

    void push(const T& val) {
        // if expand
        if (m_size >= m_capacity) {
            resize(2 * m_capacity);
        }

        m_data[m_rear++] = val;
        m_rear %= m_capacity;
        m_size++;
    }

    T top() const {
        assert(m_size > 0);
        return m_data[m_front];
    }

    T pop() {
        assert(m_size > 0);
        T ret = m_data[m_front];
        m_front = (m_front + 1) % m_capacity;
        m_size--;
        return ret;
    }

    size_t size() const { return m_size; }
    bool empty() const { return m_size != 0; }

    void print() {
        if (m_size == 0) {
            std::cout << "null\n";
            return;
        }

        for (int i = 0; i < m_size; ++i) {
            std::cout << m_data[(m_front + i) % m_capacity] << " ";
        }
        std::cout << '\n';
    }

private:
    void resize(size_t newCapacity) {
        T* new_data = new T[newCapacity];

        for (int i = 0; i < m_size; ++i) {
            new_data[i] = m_data[(m_front + i) % m_capacity];
        }
        delete[] m_data;
        m_data = new_data;
        new_data = nullptr;

        m_capacity = newCapacity;
        m_front = 0;
        m_rear = m_size;
    }

    T* m_data;
    size_t m_capacity;
    size_t m_front;
    size_t m_rear;
    size_t m_size;
};

int main() {

    CircleQueue<int> que(4);

    que.push(1);
    que.push(2);
    que.push(3);
    que.push(4);

    que.print();

    que.pop();
    que.pop();

    que.push(5);
    que.push(6);
    que.push(7);
    que.print();
    que.pop();
    que.pop();

    que.print();

    return 0;
}