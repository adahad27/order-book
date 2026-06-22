#pragma once
#include <string>
enum class OrderType {
    BID,
    ASK
};


struct Order {
    OrderType order_type;
    uint32_t quantity;
    std::string user_id;
};