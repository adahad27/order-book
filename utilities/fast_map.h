
/*
NOTE:

There is a more optimal way of designing this class, where instead of
representing both the bid book and the ask book separately, they are represented
in the same map, and pointers are used to distinguish the tops of the books.
I wrote this as a drop in replacement because I wanted to swap the
implementations in and out to benchmark them against each other

So this class was intended to be a drop in replacement for std::map.



Instead of using an RB tree, it instead uses a flat array for better cache
locality.

There is a granularity of tick-size, where we statically map all possible prices
over the tick-size.

So assuming the tick-size is 0.1
Then our price levels would look like this:

[... 198.7 198.8 198.9 190.0 190.1 ...]

To find the best bid/ask we will use a pointer.

*/
#include <stdexcept>
#include <vector>
#include "pool.h"

using byte = unsigned char;

template <typename T>
class FastMap {
   private:
    enum class SortType { ASCENDING, DESCENDING };

    double tick_size;
    double exp_lower;
    double exp_upper;
    std::pair<double, T>* best_rest_ptr;
    SortType sort_type;
    size_t _size;
    std::vector<std::vector<std::pair<double, T>>> data;
    std::vector<byte> bitmap;

    inline size_t calc_idx(double key);
    inline bool get_bitmap(double key);
    inline void set_bitmap(double key, bool bit);

   public:
    FastMap() = delete;
    FastMap(double tick, double lower_bound, double upper_bound, SortType sort)
        : tick_size(tick),
          exp_lower(lower_bound),
          exp_upper(upper_bound),
          _size(0),
          sort_type(sort),
          best_rest_ptr(nullptr),
          data((upper_bound - lower_bound) / tick),
          bitmap((upper_bound - lower_bound) / (8 * tick)) {}

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

    /*
    insert() and emplace() don't make sense
    because all of the values in the vector
    will have been default constructed
    already.

    Therefore we forbid the user from inserting
    a value. So this interface that would normally
    be supported is not.
    */
    // const std::pair<double, T>* insert(double key, T value);
    // const std::pair<double, T>* emplace(double key, T value);
    const std::pair<double, T>* find(double key);
    bool erase(double key);

    const T& at(double key);
    T& operator[](double key);
};

template <typename T>
inline size_t FastMap<T>::calc_idx(double key) {
    return (key - exp_lower) / tick_size;
}

template <typename T>
inline bool FastMap<T>::get_bitmap(double key) {
    size_t idx = calc_idx(key);
    size_t chunk = idx >> 3;
    byte chunk_offset = idx & (0b111);

    return bitmap[chunk] & (1 << chunk_offset);
}

template <typename T>
inline void FastMap<T>::set_bitmap(double key, bool bit) {
    size_t idx = calc_idx(key);
    size_t chunk = idx >> 3;
    byte chunk_offset = idx & (0b111);
    byte mask;
    if (bit) {
        // bitwise OR with all bits set to 0 except for chunk offset
        mask = 1 << chunk_offset;
        bitmap[chunk] |= mask;
    } else {
        // bitwise AND with all bits set to 1 except for chunk offset
        mask = ~(1 << chunk_offset);
        bitmap[chunk] &= mask;
    }
}

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

template <typename T>
const std::pair<double, T>* FastMap<T>::find(double key) {
    // check bitmap if value exists else return nullptr
    return get_bitmap(key) ? data.data() + calc_idx(key) : nullptr;
}

template <typename T>
const T& FastMap<T>::at(double key) {
    if (get_bitmap(key)) return data[calc_idx(key)].second;

    throw std::runtime_error("Key does not exist!");
}

// TODO: Can we use concepts to force .clear()?
template <typename T>
bool FastMap<T>::erase(double key) {
    // set bit pointed to by key to false
    if(!get_bitmap(key)) {
        return false;
    }
    set_bitmap(key, false);
    // update best_rest_ptr if necessary

    return true;
}

template <typename T>
T& FastMap<T>::operator[](double key) {
    // set bit pointed to by key to true
    

    set_bitmap(key, true);

    size_t idx = calc_idx(key);

    // update best_rest_ptr if necessary
    if ((sort_type == SortType::ASCENDING &&
         best_rest_ptr - data.data() < idx) ||
        (sort_type == SortType::DESCENDING &&
         best_rest_ptr - data.data() > idx)) {
        best_rest_ptr = data.data() + idx;
    }

    return data[calc_idx(key)];
}