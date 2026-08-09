#pragma once
#include <stdexcept>


constexpr uint32_t MAX_LIST_ALLOC_SIZE = 1 << 10;

/*
Hook that all objects which need to be stored in a linked list must
inherit from. This will allow for the List class to enforce intrusiveness
similar to the Boost implementation.
*/
struct ILLHook {
    ILLHook* next{nullptr};
    ILLHook* prev{nullptr};
};


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



template <typename T>
concept ILLHookDerived = std::derived_from<T, ILLHook>;




template <ILLHookDerived T>
class List {
private:
    
    uint32_t _size;
    Pool<T> pool;
    T* head {nullptr};
    T* tail {nullptr};
public:
    List() : pool(MAX_LIST_ALLOC_SIZE) {}

    bool push_back(const T& obj) {
        T* new_node = pool.alloc_obj();
        *new_node = obj;
        
        if (!new_node) return false;

        if(_size == 0) {
            head = new_node;
            tail = new_node;
            
        } else {
            tail->next = new_node;
            new_node->prev = tail;
            tail = new_node;
        }

        ++_size;
        return true;
    }

    bool push_front(const T& obj) {
        T* new_node = pool.alloc_obj();
        *new_node = obj;
        
        if (!new_node) return false;

        if(_size == 0) {
            head = new_node;
            tail = new_node;
            
        } else {
            head->prev = new_node;
            new_node->next = head;
            head = new_node;
        }
        ++_size;
        return true;
    }

    void pop_front() {
        if(_size > 0) {
            T* front_node = head;
            head = reinterpret_cast<T*>(head->next);
            if(head) {
                head->prev = nullptr;
            }
            
            front_node->next = nullptr;
            pool.free_obj(front_node);
            --_size;
        }
    }

    void clear() {
        while(_size > 0) {
            pop_front();
        }
    }

    void erase(T* obj) {
        if(!obj) {
            return;
        }

        if(obj == head) {
            head = reinterpret_cast<T*>(obj->next);
            head->prev = nullptr;
        } else if (obj == tail) {
            tail = reinterpret_cast<T*>(obj->prev);
            tail->next = nullptr;
        } else {
            T* prev_node = reinterpret_cast<T*>(obj->prev);
            T* next_node = reinterpret_cast<T*>(obj->next);
            prev_node->next = next_node;
            next_node->prev = prev_node;
            
        }
        obj->next = nullptr;
        obj->prev = nullptr;

        pool.free_obj(obj);
        --_size;
    }

    uint32_t size() const {
        return _size;
    }

    bool empty() const {
        return _size == 0;
    }

    const T& front() const {
        return *head;
    }

    const T& back() const {
        return *tail;
    }

    T& front() {
        return *head;
    }

    T& back() {
        return *tail;
    }

    T* first() {
        return head;
    }

    T* last() {
        return tail;
    }

};
