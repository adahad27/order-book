#include "queue.h"
#include <stdlib.h>
#include <cassert>
#include <iostream>
#include <boost/thread.hpp>
#include <random>

SPSCQueue<int> q(8);

constexpr uint64_t ITERATIONS = 1e+9;
constexpr uint8_t YIELD_PERCENTAGE = 5;

void producer() {
    for(uint64_t i = 0; i < ITERATIONS;) {
        if(q.write(i)) {
            i++;
        }
        uint8_t percentage = rand() % 100;
        if(0 <= percentage && percentage < YIELD_PERCENTAGE) {
            boost::this_thread::yield();
        }
    }
}


void consumer() {
    uint64_t expected = 0;
    while(expected < ITERATIONS) {
        std::optional<int> val = q.read();

        if(val.has_value()) {
            if(val.value() != expected) {
                std::cout << "Consumer expected " << expected << " but received " << val.value() << std::endl;
                assert(false);
            }
            expected++;
        }
        uint8_t percentage = rand() % 100;
        if(0 <= percentage && percentage < YIELD_PERCENTAGE) {
            boost::this_thread::yield();
        }
    }
}

int main() {

    std::cout << "TESTING DEFAULT QUEUE IMPLEMENTATION\n";
    Queue<int> test_queue_1;

    // Handles checking basic pushing/popping
    for(size_t i = 0; i < 8; ++i) {
        test_queue_1.push_back(i);
        assert(i == test_queue_1.back());
        assert(test_queue_1.size() == i + 1);
    }
    
    for(size_t i = 0; i < 8; ++i) {
        assert(i == test_queue_1.front());
        test_queue_1.pop_front();
        assert(test_queue_1.size() == 7 - i);
    }

    std::cout << "TEST PASSED: Basic pushing/popping front()/back()...\n";

    // Handles checking basic pushing/popping to an intialized container
    for(size_t i = 0; i < 16; ++i) {
        test_queue_1.push_back(i);
        assert(i == test_queue_1.back());
    }
    
    for(size_t i = 0; i < 16; ++i) {
        assert(i == test_queue_1.front());
        test_queue_1.pop_front();
    }

    std::cout << "TEST PASSED: pushing/popping front()/back() to initalized container...\n";

    std::cout << std::endl;

    std::cout << "TESTING SINGLE THREADED IMPLEMENTATION FOR LOCK FREE QUEUE\n";
    
    // Handles checking basic pushing/popping
    for(size_t i = 0; i < 8; ++i) {
        q.write(i);
    }
    
    for(size_t i = 0; i < 8; ++i) {
        assert(q.read().value() == i);
    }

    std::cout << "TEST PASSED: Basic pushing/popping for lock free queue...\n";

    
    assert(!q.read().has_value());

    std::cout << "TEST PASSED: Reading when queue is empty...\n";

    for(size_t i = 0; i < 8; ++i) {
        q.write(i);
    }

    assert(!q.write(8));

    std::cout << "TEST PASSED: Writing when queue is full...\n";

    for(size_t i = 0; i < 4; ++i) {
        q.read(); //0 - 3 should no longer be in queue
    }

    for(size_t i = 8; i < 12; ++i) {
        q.write(i); //queue should contain 4-11
    }

    for(size_t i = 4; i < 12; ++i) {
        assert(q.read().value() == i);
    }

    std::cout << "TEST PASSED: Testing wrap around for queue...\n";

    std::cout << "TESTING SINGLE PRODUCER SINGLE CONSUMER IMPLEMENTATION FOR SPSC Queue\n";

    boost::thread producer_thread{producer};
    boost::thread consumer_thread{consumer};

    producer_thread.join();
    consumer_thread.join();

    std::cout << "TEST PASSED: Multiple Threads...\n";



}