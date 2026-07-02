#include "ledger.h"


uint32_t Ledger::add_order(const Order &order) {

    if (order.order_type == OrderType::ASK) {
        uint32_t outstanding = order.quantity;
        while(order.price <= bid_book[order.ticker].begin()->first) {
            while(outstanding > 0) {
                /*
                If queue.empty() -> we remove this price level from the map

                If outstanding >= queue.front().quantity -> pop item from queue, outstanding -= queue.front().quantity
                If oustanding < queue.front().quantity -> modify queue.front().quantity, oustanding = 0
                |
                +-> Add to record history if order is executed 
                */
                double price = bid_book[order.ticker].begin()->first;
                std::deque<Order> &order_queue = bid_book[order.ticker].begin()->second;
                if(order_queue.empty()) {
                    bid_book[order.ticker].erase(price);
                    break;
                }
                if(outstanding >= order_queue.front().quantity) {
                    outstanding -= order_queue.front().quantity;
                    //TODO: Add to event_history about price matching
                    order_queue.pop_front();                    
                } else {
                    outstanding = 0;
                    //TODO: Add to event_history about price matching
                }
            }
        }
        ask_book[order.ticker][order.price].push_back(order);
    } else {
        bid_book[order.ticker][order.price].push_back(order);
    }

    return 0;
}