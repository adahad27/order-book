#pragma once
#include <cstdint>
#include <cstring>
#include <iostream>
#include <deque>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <stdexcept>
#include <vector>
#include <atomic>

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
/*
Queue Design:

We are going to keep two different pointers, the head pointer and the tail pointer.
These pointers are going to point at the nex valid location to read and write from.


Writing:
We cannot move the tail pointer until we are completely done with our write.
If we move our tail pointer first and then try writing, then in between those two events,
the consumer can read, it will see the tail pointer has moved, and will assume that the
value at tail is valid even though the write is incomplete.

Reading:
Consider a similar situation, but this time it would happen in a slightly different case where
the write has wrapped around the back of the buffer. If you update your head pointer first
before completing the read, the write will see there is a free space and attempt to write to
that spot in the buffer corrupting the read in progress.

Consider the alternative, which is to write/read the data before updating the pointer.

Writing an element to the tail pointer:
Writing the data and then atomically updating the tail pointer, so consider the situation where
we are partially done writing the data, and the consumer tries reading, well since we haven't
updated our head, the read could never consume the element that we are currently writing to
as long as reads/writes don't get reordered. To ensure the ordering, we must have that our store
on the tail pointer is a Memory_Order_Release.

Reading an element from the head pointer:
Reading the data and then atomically updating the head pointer, so again consider the situation
where we are partially done reading the data, and the producer tries writing, since we haven't
updated our tail, the write could never occur on the element that we are trying to read on. Again
to ensure the ordering, we must have that our store on the head pointer is a Memory_Order_Release.

Now before we start doing operations we must confirm where the head and tail pointers are, so we
will have to read those variables. We cannot allow for any read/write operations after those reads
to get reordered before. So when reading the head/tail pointers, that load must be Memory_Order_Acquire.


*/
template <typename T>
class LocklessQueue {
private:

    std::vector<T> data;

    alignas(64) uint64_t head;
    alignas(64) uint64_t tail;

public:

    LocklessQueue(uint32_t capacity) : data(capacity) {
        if(capacity == 0) {
            throw std::runtime_error("Cannot have queue of capacity 0");
        } else if (capacity & (capacity - 1) != 0) {
            throw std::runtime_error("Must set queue to have capacity of power of 2")
        }
    }

    void write(const T& v) {
        
    }

    T read() {

    }


};

template <typename T>
class WaitQueue {
private:
    std::deque<T> queue;
    boost::mutex m;
    boost::condition_variable cv;
    

public:

    void push_back(const T& v) {
        boost::unique_lock<boost::mutex> lock(m);
        queue.push_back(v);
        cv.notify_one();
    }

    T pop_front() {
        boost::unique_lock<boost::mutex> lock(m);
        while(queue.empty()) {
            cv.wait(lock);
        }

        T val = queue.front();
        queue.pop_front();
        return val;
    }

    const T& front() {
        boost::unique_lock<boost::mutex> lock(m);
        return queue.front();
    }

    const T& back() {
        boost::unique_lock<boost::mutex> lock(m);
        return queue.back();
    }

    uint32_t size() {
        boost::unique_lock<boost::mutex> lock(m);
        return queue.size();
    }
};