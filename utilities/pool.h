#pragma once
#include <stdexcept>

template <typename T>
class Pool {
private:
    uint8_t *data;
    uint32_t num_obj;

public:

    //Must pass in number of objects to allocate at construction time
    Pool() = delete;

    Pool(uint32_t num_objects);

    uint8_t* alloc_obj();

    void free_obj(uint8_t*& obj);

    ~Pool();
};