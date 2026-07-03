#pragma once
#include <map>
#include <deque>
#include <string>
#include <unordered_map>
#include <vector>
#include "types.h"


class Ledger {
private:
    // Ticker -> Map(Price -> Order)
    std::unordered_map<std::string, std::map<double, std::deque<Order>, std::greater<double>>> bid_book{}; //max-heap
    std::unordered_map<std::string, std::map<double, std::deque<Order>>> ask_book{}; //min-heap

    std::unordered_map<uint32_t, Order*> outstanding_orders; //need to use smart pointers here

    std::vector<Event> event_history;

    uint32_t global_order_id;

public:

    Ledger() : global_order_id(0) {

    }

    uint32_t add_order(Order order);

    void print_events();

    bool cancel_order(uint32_t order_id);

    bool modify_order(uint32_t order_id, const Order &order);

    const Order& get_best_ask() const;

    const Order& get_best_bid() const;

    //T get_spread() const;

};