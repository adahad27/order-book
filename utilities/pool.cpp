#include "pool.h"

template <typename T>
Pool<T>::Pool(uint32_t num_objects) : num_obj(num_objects) {
    data = new T[num_objects];
    if(!data) {
        throw std::runtime_error("Unable to allocate Pool on heap")
    }
}

template <typename T>
uint8_t* Pool<T>::alloc_obj() {

}

template <typename T>
void Pool<T>::free_obj(uint8_t*& obj) {

    //invalidate the pointer for the user
    obj = nullptr;
}

template <typename T>
Pool<T>::~Pool() {
    delete[] data;
}