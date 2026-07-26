#include "fast_map.h"
#include <iostream>
#include <cassert>

int main() {
    FastMap<int> map(0.1, 40, 80, FastMap<int>::SortType::ASCENDING);


    assert(map.empty());
    map[50.0] = 1;
    assert(map.begin()->first == 50.0);
    assert(map.begin()->second == 1);

    map[50.1] = 2;
    assert(map.begin()->first == 50.1);
    assert(map.begin()->second == 2);

    map[70.1] = 3;
    assert(map.begin()->first == 70.1);
    assert(map.begin()->second == 3);

    map[40.3] = 4;
    assert(map.begin()->first == 70.1);
    assert(map.begin()->second == 3);

    map.erase(70.1);
    assert(map.begin()->first == 50.1);
    assert(map.begin()->second == 2);

    map.erase(50.0);
    assert(map.begin()->first == 50.1);
    assert(map.begin()->second == 2);

    map.erase(50.1);
    assert(map.begin()->first == 40.3);
    assert(map.begin()->second == 4);
    
}