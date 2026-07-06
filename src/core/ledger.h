#pragma once
#include <map>
#include <list>
#include <string>
#include <unordered_map>
#include <vector>
#include "types.h"
#include <optional>
#include <functional>
#include <deque>
#include "queue.h"
#include "boost/thread.hpp"
#include "pool.h"

class Tester;

class Ledger {
private:

    friend class Tester;
    // Ticker -> Map(Price -> Order)
    std::unordered_map<std::string, std::map<double, std::list<Order>, std::greater<double>>> bid_book{}; //max-heap
    std::unordered_map<std::string, std::map<double, std::list<Order>>> ask_book{}; //min-heap

    std::unordered_map<uint32_t, OrderEntry> outstanding_orders; //need to use smart pointers here

    std::vector<Event> event_history;

    uint32_t global_order_id;

    WaitQueue<Job> &m_req_queue;
    WaitQueue<uint32_t> &m_resp_queue;

    void resolve_order(auto &book, Order &order);

    uint32_t add_order_id(Order order, std::optional<uint32_t> order_id);

    void cancel_order_helper(uint32_t order_id);

    void start_loop();

    uint32_t add_order(Order order);

    bool cancel_order(uint32_t order_id);

    //Only able to change price/quantity
    bool modify_order(uint32_t order_id, Order order);

public:


    Ledger(WaitQueue<Job> &req_queue, WaitQueue<uint32_t> &resp_queue) : 
    global_order_id(0), m_req_queue(req_queue), m_resp_queue(resp_queue) {}

    void start_ledger();

    

    //TODO: Move this to another piece of code
    void print_events();

    const Order& get_best_ask() const;

    const Order& get_best_bid() const;

    //T get_spread() const;

};