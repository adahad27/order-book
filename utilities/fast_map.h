
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

Supporting:
.begin() -> supporting finding the top of the book
.erase() -> support deleting from anywhere in the book
[] operator -> support insertion if not existing, support access if existing
.find() -> support access if existing





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
    long best_rest_ptr;
    SortType sort_type;
    std::vector<std::vector<std::pair<double, T>>> data;
    std::vector<byte> bitset

public:
    
    FastMap() = delete;
    FastMap(double tick, double lower_bound, double upper_bound, SortType sort) :
    tick_size(tick), exp_lower(lower_bound), exp_upper(upper_bound), sort_type(sort),
    data((upper_bound - lower_bound)/tick), bitset((upper_bound - lower_bound)/(8*tick)) {}

    
    
    void erase(double price) {
        //set bitmask off, don't deallocate memory
    }

    T& operator[](double price) {
        //check if price exists, if not allocate
        //then return value
    }

    std::pair<double, T>* begin() {
        //return object at the top of the book
    }

    std::pair<double, T>* find(double price) {
        //return object pointed to by this price using bin search?
    }





};