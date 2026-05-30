#pragma once

template<typename T>
class Queue {
private:
    T *data;
    uint32_t capacity;
    uint32_t size;
    uint32_t head;
    uint32_t tail;

    void grow() {
        capacity = capacity == 0 ? 1 : capacity * 2;
        T *new_data = new T[capacity];
        memcpy(new_data, data + head, (size - head + 1) * sizeof(T));
        memcpy(new_data + size - head + 1, data, )
    }

public:
    Queue() : data(nullptr), capacity(0), size(0), head(0), tail(0) {}

    const T& front() {
        return data[head];
    }

    const T& back() {
        return data[tail - 1];
    }

    void push_back(const T& v) {
        if(capacity == size) {
            //grow the queue
            grow();
        }
        data[tail] = v;
        size++;
        tail = (tail + 1) % capacity;
    }

    ~Queue() {
        if (data) {
            delete[] data;
        }
    }
};