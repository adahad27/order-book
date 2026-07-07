#include "pool.h"
#include <iostream>
#include <cassert>
int main() {


    Pool<int> integer_pool(10);

    int* arr[11];

    int k = 10;
    for(int i = 0; i < 10; ++i) {
        k--;
        arr[i] = integer_pool.alloc_obj();
        *(arr[i]) = i;
        assert(integer_pool.get_available() == k);
    }

    arr[10] = integer_pool.alloc_obj();
    assert(arr[10] == nullptr);

    

    integer_pool.free_obj(arr[0]);
    integer_pool.free_obj(arr[1]);

    int *a = integer_pool.alloc_obj();
    int *b = integer_pool.alloc_obj();

    assert(a != b);
    







}