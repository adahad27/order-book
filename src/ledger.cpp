#include "ledger.h"
/*
REQUIRES: 
order must have been price matched to a resting order in book.

MODIFIES: 
order.quantity, book[order.ticker][price]->queue.

EFFECT: 
DOes price-time matching on the incoming order against resting orders in opposite book.
Will match most favorable trade until all quantity is exhausted, before moving onto next most favorable trade.
*/
template <typename Compare>
void resolve_order(std::unordered_map<std::string, std::map<double, std::deque<Order>, Compare>> &book, std::vector<Event> &event_history, Order &order) {
    while(order.quantity > 0) {
        /*
        If queue.empty() -> we remove this price level from the map

        If outstanding >= queue.front().quantity -> pop item from queue, outstanding -= queue.front().quantity
        If outstanding < queue.front().quantity -> modify queue.front().quantity, outstanding = 0
        |
        +-> Add to record history if order is executed 
        */
        double price = book[order.ticker].begin()->first;
        std::deque<Order> &order_queue = book[order.ticker].begin()->second;
        if(order_queue.empty()) {
            book[order.ticker].erase(price);
            return;
        }
        
        Event matched_order = Event{.event_type = EventType::ORDER_EXEC, .order = Order{order}};
        matched_order.order.price = order_queue.front().price;
        matched_order.order.quantity = std::min(order_queue.front().quantity, matched_order.order.quantity);
        event_history.emplace_back(matched_order);

        if(order.quantity >= order_queue.front().quantity) {
            order.quantity -= order_queue.front().quantity;
            order_queue.pop_front();
            
        } else {       
            order_queue.front().quantity -= order.quantity;            
            order.quantity = 0;
            
        }
    }
}

uint32_t Ledger::add_order(Order order) {
    
    if (order.order_type == OrderType::ASK) {
        while(bid_book.contains(order.ticker) && order.price <= bid_book[order.ticker].begin()->first) {
            resolve_order(bid_book,event_history, order);
            if(order.quantity == 0) {
                break;
            }
        }
        if(order.quantity > 0) {
            ask_book[order.ticker][order.price].push_back(order);
        }        
    } else {
        while(ask_book.contains(order.ticker) && order.price >= ask_book[order.ticker].begin()->first) {
            resolve_order(ask_book,event_history, order);
            if(order.quantity == 0) {
                break;
            }
        }
        if(order.quantity > 0) {
            bid_book[order.ticker][order.price].push_back(order);
        }
        
    }

    global_order_id++;
    return global_order_id;
}