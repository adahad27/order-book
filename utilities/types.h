#pragma once
#include <string>
#include <iostream>
#include <list>
#include <map>
enum class OrderType {
    BID,
    ASK
};

enum class OrderSubType {
    MARKET, // buy/sell at whatever the best current price is on the opposite side
    LIMIT, // buy/sell at the limit that is set in the order or a better price
    STOP_LOSS, // sell at best market price if the last traded price is at or below some level
    FILL_OR_KILL, // fill the position in it's entirety if possible, else cancel it
    IMMED_OR_CANCEL //fill or partially fill the order if possible, else cancel it
};

enum class EventType {
    INC_SELL,
    INC_BUY,
    ORDER_EXEC
};

enum class JobType {
    ADD,
    CANCEL,
    MODIFY
};

struct Order {
    std::string ticker;
    std::string user_id;
    uint32_t order_id;
    OrderType order_type;
    OrderSubType order_subtype;
    double price;
    uint32_t quantity;    
};

struct FilledOrder {
    std::string ticker;
    std::string bidder_id;
    std::string asker_id;
    uint32_t bid_order_id;
    uint32_t ask_order_id;
    double price;
    uint32_t quantity;
};

struct Job {
    JobType job_type;
    uint32_t order_id;
    Order order;
};

struct OrderEntry {
    std::map<double, std::list<Order>>::iterator entry_list;
    std::list<Order>::iterator entry;
};

struct Event {
    EventType event_type;
    FilledOrder filled_order;
};

inline bool operator==(Order &order_l, Order &order_r) {
    return (
        order_l.ticker == order_r.ticker &&
        order_l.user_id == order_r.user_id &&
        order_l.order_type == order_r.order_type &&
        order_l.order_subtype == order_l.order_subtype &&
        order_l.price == order_r.price &&
        order_l.quantity == order_r.quantity
    );
}

//TODO: Fix this to be proper
inline std::ostream& operator<<(std::ostream &os, const Event& event) {
    switch(event.event_type) {
        case EventType::INC_BUY:
            os << "INCOMING BUY";
            break;
        case EventType::INC_SELL:
            os << "INCOMING SELL";
            break;
        case EventType::ORDER_EXEC:
            os << "ORDER EXECUTED";
            break;
    }
    std::cout << "\n";
    std::cout <<"\tTicker: " << event.filled_order.ticker << "\n";
    std::cout <<"\tPrice: " << event.filled_order.price << "\n";
    std::cout <<"\tQuantity: " << event.filled_order.quantity << "\n";
    std::cout << std::endl; //flush and add another linebreak
    return os;
}
