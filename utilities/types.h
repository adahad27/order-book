#pragma once
#include <string>
enum class OrderType {
    BID,
    ASK
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
    double price;
    uint32_t quantity;    
};

struct Event {
    EventType event_type;
    Order order;
};
