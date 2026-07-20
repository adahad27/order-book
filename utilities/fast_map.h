
/*
NOTE:

There is a more optimal way of designing this class, where instead of
representing both the bid book and the ask book separately, they are represented
in the same map, and pointers are used to distinguish the tops of the books.
I wrote this as a drop in replacement because I wanted to swap the implementations
in and out to benchmark them against each other

So this class was intended to be a drop in replacement for std::map.



Instead of using an RB tree, it instead uses a flat array for better cache locality.

There is a granularity of tick-size, where we statically map all possible prices
over the tick-size.

So assuming the tick-size is 0.1
Then our price levels would look like this:

[... 198.7 198.8 198.9 190.0 190.1 ...]

To find the best bid/ask we will use a pointer.

*/
#include "pool.h"
#include <vector>
// #include <bitset>

using byte = unsigned char;

template <typename T>
class FastMap {

    

private:
    enum class SortType {
        ASCENDING,
        DESCENDING
    };

    double tick_size;
    double exp_lower;
    double exp_upper;
    std::pair<double, T>* best_rest_ptr;
    SortType sort_type;
    size_t _size;
    std::vector<std::vector<std::pair<double, T>>> data;
    std::vector<byte> bitset

public:
    
    FastMap() = delete;
    FastMap(double tick, double lower_bound, double upper_bound, SortType sort) :
    tick_size(tick), exp_lower(lower_bound), exp_upper(upper_bound), _size(0), sort_type(sort),
    best_rest_ptr(nullptr), data((upper_bound - lower_bound)/tick), 
    bitset((upper_bound - lower_bound)/(8*tick)) {}

    
    const std::pair<double, T>* begin();
    
    /*
    end() not supported for now because
    iteration over non-empty buckets is
    a non-trivial task.

    Disallow user from iterating by just
    not letting them know what the end()
    of the structure is.
    */
    // const std::pair<double, T>* end();
    bool empty();
    size_t size();

    const std::pair<double, T>* insert(double key);
    const std::pair<double, T>* emplace(double key);
    const std::pair<double, T>* find(double key);

    const T& at();
    T& operator[]();



};

/*
Burden on user to make sure that begin()
points to something valid and not the
nullptr
*/
template <typename T>
const std::pair<double, T>* begin() {
    return best_rest_ptr;
}

template <typename T>
bool FastMap<T>::empty() {
    return !_size;
}

template <typename T>
size_t FastMap<T>::size() {
    return _size;
}

