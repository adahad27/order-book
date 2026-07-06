#pragma once
#include <stdexcept>

template <typename T>
class Pool {
private:
    T *data;
    T **free_list;
    uint32_t num_obj;
    
    T **list_top;

public:

    //Must pass in number of objects to allocate at construction time
    Pool() = delete;

    Pool(uint32_t num_objects);

    T* alloc_obj();

    void free_obj(T*& obj);

    ~Pool();
};