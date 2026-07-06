#include "pool.h"

template <typename T>
Pool<T>::Pool(uint32_t num_objects) : num_obj(num_objects) {
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

template <typename T>
T* Pool<T>::alloc_obj() {
    if(list_top < free_list) {
        return nullptr;
    }
    return *list_top--;

}

template <typename T>
void Pool<T>::free_obj(T*& obj) {

    //Must confirm that passed in pointer belongs to our pool
    if(obj < data || obj >= data + num_obj) {
        return;
    }

    *list_top++ = obj;
    obj = nullptr;


}

template <typename T>
Pool<T>::~Pool() {
    delete[] data;
    delete[] free_list;
}