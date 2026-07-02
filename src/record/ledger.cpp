#include "ledger.h"


uint32_t Ledger::add_order(Order &order) {

    if (order.order_type == OrderType::ASK) {
        while(order.price <= bid_book[order.ticker].begin()->first) {
            while(order.quantity > 0) {
                /*
                If queue.empty() -> we remove this price level from the map

                If outstanding >= queue.front().quantity -> pop item from queue, outstanding -= queue.front().quantity
                If outstanding < queue.front().quantity -> modify queue.front().quantity, outstanding = 0
                |
                +-> Add to record history if order is executed 
                */
                double price = bid_book[order.ticker].begin()->first;
                std::deque<Order> &order_queue = bid_book[order.ticker].begin()->second;
                if(order_queue.empty()) {
                    bid_book[order.ticker].erase(price);
                    break;
                }
                if(order.quantity >= order_queue.front().quantity) {
                    
                    //TODO: Add to event_history about price matching
                    Event matched_order = Event{.event_type = EventType::ORDER_EXEC, .order = Order{order}};
                    matched_order.order.quantity = order_queue.front().quantity;
                    matched_order.order.price = order_queue.front().price;
                    
                    event_history.emplace_back(matched_order);

                    order_queue.pop_front();
                    order.quantity -= order_queue.front().quantity;                   
                } else {

                    //TODO: Add to event_history about price matching
                    Event matched_order = Event{.event_type = EventType::ORDER_EXEC, .order = Order{order}};
                    matched_order.order.price = order_queue.front().price;
                    
                    event_history.emplace_back(matched_order);

                    order.quantity = 0;
                    order_queue.front().quantity -= order.quantity;
                }
            }
        }
        if(order.quantity > 0) {
            ask_book[order.ticker][order.price].push_back(order);
        }        
    } else {
        bid_book[order.ticker][order.price].push_back(order);
    }

    return 0;
}