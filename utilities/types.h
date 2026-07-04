#pragma once
#include <string>
#include <iostream>
#include <list>
enum class OrderType {
    BID,
    ASK
};

enum class OrderSubType {
    MARKET, // buy/sell at whatever the best current price is on the opposite side
    LIMIT, // buy/sell at the limit that is set in the order or a better price
    STOPLOSS // sell at best market price if the last traded price is at or below some level
};

enum class EventType {
    INC_SELL,
    INC_BUY,
    ORDER_EXEC
};

struct Order {
    std::string ticker;
    std::string user_id;
    OrderType order_type;
    OrderSubType order_subtype;
    double price;
    uint32_t quantity;    
};

struct OrderEntry {
    std::map<double, std::list<Order>>::iterator entry_list;
    std::list<Order>::iterator entry;
};

struct Event {
    EventType event_type;
    Order order;
};

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
    std::cout <<"\tTicker: " << event.order.ticker << "\n";
    std::cout <<"\tUser: " << event.order.user_id << "\n";
    std::cout <<"\tPrice: " << event.order.price << "\n";
    std::cout <<"\tQuantity: " << event.order.quantity << "\n";
    std::cout << std::endl; //flush and add another linebreak
    return os;
}
