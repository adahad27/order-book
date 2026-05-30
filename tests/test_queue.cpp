#include "queue.h"
#include <stdlib.h>
#include <cassert>
#include <iostream>

int main() {

    
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

    


}