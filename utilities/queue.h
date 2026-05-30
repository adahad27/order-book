#pragma once
#include <cstdint>
#include <cstring>
#include <iostream>

template <typename T>
class Queue {
   private:
    T* data;
    uint32_t _capacity;
    uint32_t _size;
    uint32_t head;
    uint32_t tail;

    void grow() {
        // Special case for if starting off with no items
        _capacity = _capacity == 0 ? 1 : _capacity * 2;
        T* new_data = new T[_capacity];

        if (data) {
            // Copy items from the front of the queue to start of new buffer
            memcpy(new_data, data + head, (_size - head) * sizeof(T));
            memcpy(new_data + _size - head + 1, data, head * sizeof(T));
            delete[] data;
        }
        data = new_data;

        head = 0;
        tail = _size;
    }

   public:
    Queue() : data(nullptr), _capacity(0), _size(0), head(0), tail(0) {}

    const T& front() { return data[head]; }

    const T& back() { return data[(tail - 1) % _capacity]; }

    void push_back(const T& v) {
        if (head == tail && _size == _capacity) {
            // grow the queue
            grow();
        }
        data[tail] = v;
        _size++;
        tail = (tail + 1) % _capacity;
    }

    void pop_front() {
        if (_size > 0) {
            _size--;
            head = (head + 1) % _capacity;
        }
    }

    uint32_t size() { return _size; }

    ~Queue() {
        if (data) {
            delete[] data;
        }
    }
};