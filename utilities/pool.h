#pragma once
#include <stdexcept>

/*
Ideas to implement:
1.) Prevent double frees -> Use a bitmap to track which address offsets are free. The size of the
bitmap is (num_objects / 8), so relatively efficient memory storage
2.) Making the pool thread safe -> We could use a mutex to just lock/unlock around the data structure.
Or we could use atomic read-modify-writes to implement a lock-free stack.

*/

template <typename T>
class Pool {
private:
    T *data;
    T **free_list;
    uint32_t num_obj;
    
    /*
    TODO:
    Could use an unsigned integer offset, the offset
    directly represents which element you want, so
    an offset of 1 would mean the first element, so on
    and so forth.
    */
    T **list_top;

public:

    //Must pass in number of objects to allocate at construction time
    Pool() = delete;

    Pool(uint32_t num_objects) : num_obj(num_objects) {

        if(num_objects == 0) {
            throw std::runtime_error("Cannot have pool of size 0");
        }

        data = new T[num_objects];
        if(!data) {
            throw std::runtime_error("Unable to allocate Pool on heap");
        }
        free_list = new T*[num_objects];
        if(!free_list) {
            throw std::runtime_error("Unable to allocate Free List on heap");
        }
        //Instantiate free_list
        for(uint32_t i = 0; i < num_obj; ++i) {
            free_list[i] = data + i;
        }

        list_top = free_list + num_obj - 1;
    }

    T* alloc_obj() {
        if(list_top < free_list) {
            return nullptr;
        }
        return *list_top--;
    }

    uint32_t get_available() {
        if(list_top < free_list) {
            return 0;
        }
        return list_top - free_list + 1;
    }

    void free_obj(T*& obj) {
        //Must confirm that passed in pointer belongs to our pool
        if(obj < data || obj >= data + num_obj) {
            return;
        }

        *++list_top = obj;
        obj = nullptr;
    }

    ~Pool() {
        //Reset free-list before we delete everything
        for(uint32_t i = 0; i < num_obj; ++i) {
            free_list[i] = data + i;
        }
        delete[] free_list;
        delete[] data;
        
    }
};