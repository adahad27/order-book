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

void Ledger::resolve_order(auto &book, Order &order) {
    
    
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
        
        FilledOrder filled_order;
        filled_order.ticker = order.ticker;

        if(order.order_type == OrderType::BID) {
            filled_order.bidder_id = order.user_id;
            filled_order.asker_id = order_queue.front().user_id;
            filled_order.bid_order_id = order.order_id;
            filled_order.ask_order_id = order_queue.front().order_id;
        } else {
            filled_order.asker_id = order.user_id;
            filled_order.bidder_id = order_queue.front().user_id;
            filled_order.ask_order_id = order.order_id;
            filled_order.bid_order_id = order_queue.front().order_id;
        }
        filled_order.price = order_queue.front().price;
        filled_order.quantity = std::min(order_queue.front().quantity, order.quantity);


        Event matched_order = Event{.event_type = EventType::ORDER_EXEC, .filled_order = filled_order};
        event_history.emplace_back(matched_order);

        if(order.quantity >= order_queue.front().quantity) {
            order.quantity -= order_queue.front().quantity;
            outstanding_orders.erase(order_queue.front().order_id);
            order_queue.pop_front();
            
            
        } else {       
            order_queue.front().quantity -= order.quantity;            
            order.quantity = 0;
            
        }
    }
    

    
}

uint32_t Ledger::add_order_id(Order order, std::optional<uint32_t> order_id) {
    std::string ticker{order.ticker};
    double price{order.price};
    uint32_t id;
    if(order_id.has_value()) {
        /*
        This code path is only triggered when doing a modify.
        Therefore, a response is not pushed from this function
        when called as a helper from modify.
        */
        id = order_id.value();
    } else {
        id = global_order_id++;
        m_resp_queue.push_back(id);
    }

    order.order_id = id;

    auto execute_order = [&](auto &home_book, auto &counter_book, std::function<bool(double, double)> compare) {
        if(order.order_subtype == OrderSubType::MARKET) {
            //If market order, then don't need to compare order price against resting liquidity price
            compare = [](double a, double b) {return true;};
        }
        while(counter_book.contains(ticker) && !counter_book[ticker].empty() && compare(price, counter_book[ticker].begin()->first)) {
            resolve_order(counter_book, order);
            if(order.quantity == 0) {
                break;
            }
        }
        if(order.quantity > 0) {
            home_book[ticker][price].emplace_back(order);
            outstanding_orders[id] = OrderEntry{
                home_book[ticker].find(price),
                std::prev(home_book[ticker][price].end())
            };

        }
    };

    if (order.order_type == OrderType::ASK) {
        execute_order(ask_book, bid_book, [](double a, double b) {return a <= b;});       
    } else {
        execute_order(bid_book, ask_book, [](double a, double b) {return a >= b;});
        
    }

    return id;
}

uint32_t Ledger::add_order(Order order) {
    return add_order_id(order, std::optional<uint32_t>{});
}

void Ledger::cancel_order_helper(uint32_t order_id) {
    Order &order = *(outstanding_orders[order_id].entry);

    if(outstanding_orders[order_id].entry_list->second.size() == 1) {
        if(order.order_type == OrderType::ASK) {
            ask_book[order.ticker].erase(order.price);
        } else {
            bid_book[order.ticker].erase(order.price);
        }
    } else {
        outstanding_orders[order_id].entry_list->second.erase(outstanding_orders[order_id].entry);
    }
    //cannot do a lookup by price to find the queue to delete in because that is O(logn)
    
    
    outstanding_orders.erase(order_id);
}

bool Ledger::cancel_order(uint32_t order_id) {
    if(!outstanding_orders.contains(order_id)) {
        m_resp_queue.push_back(false);
        return false;
    } else {
        m_resp_queue.push_back(true);
    }

    cancel_order_helper(order_id);
    return true;
    
}

bool Ledger::modify_order(uint32_t order_id, Order order) {
    if(!outstanding_orders.contains(order_id)) {
        m_resp_queue.push_back(false);
        return false;
    } else {
        m_resp_queue.push_back(true);
    }

    /*
    If price is different, then we must erase the intial value from
    the price level.
    Then we must insert a new order with the modified price into the
    order book.
    */
    if(order.price != outstanding_orders[order_id].entry->price) {
        cancel_order(order_id);
        add_order_id(order, order_id);

    }
    outstanding_orders[order_id].entry->quantity = order.quantity;

    return true;

}

void Ledger::print_events() {
    for(uint64_t i = 0; i < event_history.size(); ++i) {
        std::cout <<"Event " << i << ": " << event_history[i];
    }
}

void Ledger::start_loop() {
    while(true) {
        Job job = m_req_queue.pop_front();
        switch(job.job_type) {
            case JobType::ADD:
                add_order(job.order);
                break;
            case JobType::CANCEL:
                cancel_order(job.order_id);
                break;
            case JobType::MODIFY:
                modify_order(job.order_id, job.order);
                break;
        }
    }
}

void Ledger::start_ledger() {
    boost::thread engine_thread{&Ledger::start_loop, this};
}