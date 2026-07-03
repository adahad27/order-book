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
void resolve_order(std::unordered_map<std::string, std::map<double, std::list<Order>, Compare>> &book, std::vector<Event> &event_history, Order &order) {
    while(order.quantity > 0) {
        /*
        If queue.empty() -> we remove this price level from the map

        If outstanding >= queue.front().quantity -> pop item from queue, outstanding -= queue.front().quantity
        If outstanding < queue.front().quantity -> modify queue.front().quantity, outstanding = 0
        |
        +-> Add to record history if order is executed 
        */
        double price = book[order.ticker].begin()->first;
        std::list<Order> &order_queue = book[order.ticker].begin()->second;
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
    std::string ticker{order.ticker};
    double price{order.price};

    if (order.order_type == OrderType::ASK) {
        while(bid_book.contains(ticker) && price <= bid_book[ticker].begin()->first) {
            resolve_order(bid_book,event_history, order);
            if(order.quantity == 0) {
                break;
            }
        }
        if(order.quantity > 0) {
            ask_book[ticker][price].emplace_back(order);
            outstanding_orders[global_order_id++] = OrderEntry{ask_book[ticker].find(price), std::prev(ask_book[ticker][price].end())};

        }        
    } else {
        while(ask_book.contains(ticker) && price >= ask_book[ticker].begin()->first) {
            resolve_order(ask_book,event_history, order);
            if(order.quantity == 0) {
                break;
            }
        }
        if(order.quantity > 0) {
            bid_book[ticker][price].emplace_back(order);
            outstanding_orders[global_order_id++] = OrderEntry{bid_book[ticker].find(price), std::prev(bid_book[ticker][price].end())};
        }
        
    }

    return global_order_id;
}

bool Ledger::cancel_order(uint32_t order_id) {
    if(!outstanding_orders.contains(order_id)) {
        return false;
    }

    //cannot do a lookup by price to find the queue to delete in because that is O(logn)
    outstanding_orders[order_id].entry_list->second.erase(outstanding_orders[order_id].entry);
    outstanding_orders.erase(order_id);
    
    return true;
}

bool Ledger::modify_order(uint32_t order_id, Order order) {
    if(!outstanding_orders.contains(order_id)) {
        return false;
    }

    /*
    If price is different, then we must erase the intial value from
    the price level.
    Then we must insert a new order with the modified price into the
    order book.

    Should we utilize the methods that we have already?
    */
    if(order.price != outstanding_orders[order_id].entry->price) {

    }
    outstanding_orders[order_id].entry->quantity = order.quantity;


    return true;
}

void Ledger::print_events() {
    for(uint64_t i = 0; i < event_history.size(); ++i) {
        std::cout <<"Event " << i << ": " << event_history[i];
    }
}