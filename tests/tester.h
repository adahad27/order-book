#pragma once

#include <unordered_map>
#include <string>
#include "types.h"
#include <list>
#include <map>
#include "ledger.h"


class Ledger;

class Tester {
public:

    std::unordered_map<std::string, std::map<double, std::list<Order>, std::greater<double>>> &bid_book; //max-heap
    std::unordered_map<std::string, std::map<double, std::list<Order>>> &ask_book; //min-heap

    std::unordered_map<uint32_t, OrderEntry> &outstanding_orders; //need to use smart pointers here

    Ledger &m_book;

    Tester(Ledger &book) : 
    m_book(book),
    bid_book(book.bid_book),
    ask_book(book.ask_book),
    outstanding_orders(book.outstanding_orders) {

    }

    uint32_t add_order(Order order) {
        return m_book.add_order(order);
    }

    bool cancel_order(uint32_t order_id) {
       return m_book.cancel_order(order_id);
    }

    bool modify_order(uint32_t order_id, Order order) {
        return m_book.modify_order(order_id, order);
    }

};