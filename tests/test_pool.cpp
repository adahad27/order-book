#include "pool.h"
#include <iostream>
#include <cassert>


struct TestRecord : ILLHook {
    int field;
};

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
    

    List<TestRecord> list;

    TestRecord obj1;
    obj1.field = 1;
    TestRecord obj2;
    obj2.field = 2;

    list.push_back(obj1);
    assert(list.size() == 1);
    assert(list.front()->field == 1);
    assert(list.back()->field == 1);

    list.push_back(obj2);
    assert(list.size() == 2);
    assert(list.front()->field == 1);
    assert(list.back()->field == 2);

    TestRecord* obj4;
    TestRecord* obj5;
    TestRecord* obj6;

    for(int i = 3; i <= 10; ++i) {
        TestRecord obj;
        obj.field = i;

        list.push_back(obj);

        if(i == 4) {
            obj4 = list.back();
        }
        else if(i == 5) {
            obj5 = list.back();
        } else if (i == 6) {
            obj6 = list.back();
        }

        assert(list.size() == i);
        assert(list.back()->field == i);
    }


    list.erase(obj5);
    assert(list.size() == 9);
    assert(obj4->next == obj6);
    assert(obj6->prev == obj4);
    assert(obj5->next == nullptr);
    assert(obj5->prev == nullptr);







}