#include "fast_map.h"
#include <cstdlib>
#include <iostream>
#include <stdexcept>
/*
TODO:

THIS WILL NOT COMPILE BECAUSE FASTMAP NOW
EXPECTS TO RECEIVE A CONCEPT THAT HAS THE
CLEAR() FUNCTION IMPLEMENTED.

STILL NEED TO CHANGE THE TYPE OF FASTMAP
TO BE A DIFFERENT VALUE, AND THEN REWRITE
TESTS FOR THAT

*/
namespace {

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FastMap test failed: " << message << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void test_ascending_behavior() {
    FastMap<int> map(0.1, 60.0, 20.0, SortType::ASCENDING);

    expect(map.empty(), "new ascending map should be empty");
    expect(map.size() == 0, "new ascending map should have size 0");

    map[50.0] = 1;
    expect(!map.empty(), "map should not be empty after inserting a value");
    expect(map.size() == 1, "size should increase after the first insert");
    expect(map.begin() != nullptr, "begin should point to a value after insertion");
    expect(map.begin()->first == 50.0, "ascending begin should track the first inserted key");
    expect(map.begin()->second == 1, "ascending begin should preserve the inserted value");

    map[50.1] = 2;
    expect(map.size() == 2, "size should increase after the second insert");
    expect(map.begin()->first == 50.1, "ascending begin should track the highest inserted key");
    expect(map.begin()->second == 2, "ascending begin should keep the value associated with the highest inserted key");

    const auto* found = map.find(50.1);
    expect(found != nullptr, "find should locate an inserted key");
    expect(found->second == 2, "find should return the stored value for a known key");
    expect(map.find(60.0) == nullptr, "find should return nullptr for a missing key");
    expect(map.at(50.0) == 1, "at should return the stored value for an existing key");

    bool threw = false;
    try {
        (void)map.at(60.0);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    expect(threw, "at should throw for a missing key");

    expect(map.erase(50.0), "erase should return true for an existing key");
    expect(map.size() == 1, "size should decrease after erasing one key");
    expect(map.begin()->first == 50.1, "begin should remain at the current best key after erase");
    expect(!map.erase(50.0), "erase should return false for a key that no longer exists");

    expect(map.erase(50.1), "erase should remove the remaining key");
    expect(map.empty(), "map should be empty once all keys are removed");
    expect(map.size() == 0, "size should return to 0 after removing all keys");
}

void test_descending_behavior() {
    FastMap<int> map(0.1, 40.0, 80.0, SortType::DESCENDING);

    map[50.0] = 1;
    map[50.1] = 2;
    map[70.1] = 3;
    map[40.3] = 4;

    expect(map.size() == 4, "descending map should track four inserted keys");
    expect(map.begin()->first == 40.3, "descending begin should point at the lowest inserted key");
    expect(map.begin()->second == 4, "descending begin should keep the associated value");

    expect(map.erase(70.1), "erase should remove an existing descending key");
    expect(map.begin()->first == 40.3, "begin should remain at the lowest surviving key after erase");

    expect(map.erase(40.3), "erase should remove the current best descending key");
    expect(map.begin()->first == 50.0, "begin should move to the next surviving descending key");
    expect(map.begin()->second == 1, "begin should preserve the correct value after moving");

    expect(map.erase(50.1), "erase should remove another descending key");
    expect(map.erase(50.0), "erase should remove the last descending key");
    expect(map.begin() == nullptr, "begin should return nullptr once the descending map is empty");
}



}  // namespace

int main() {
    test_ascending_behavior();
    test_descending_behavior();
    std::cout << "All FastMap tests passed" << std::endl;
    return EXIT_SUCCESS;
}