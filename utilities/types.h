#pragma once
#include <string>
#include <iostream>
#include <list>
#include <map>
#include "pool.h"

enum class OrderType : uint8_t{
    BID,
    ASK
};

enum class SortType : uint8_t { ASCENDING, DESCENDING };

enum class OrderSubType : uint8_t {
    MARKET, // buy/sell at whatever the best current price is on the opposite side
    LIMIT, // buy/sell at the limit that is set in the order or a better price
    STOP_LOSS, // sell at best market price if the last traded price is at or below some level
    FILL_OR_KILL, // fill the position in it's entirety if possible, else cancel it, these can either be done at market price or at limit price
    IMMED_OR_CANCEL //fill or partially fill the order if possible, cancel remaining balance, these can either be done at market price or at limit price
};

enum class EventType : uint8_t {
    INC_SELL,
    INC_BUY,
    ORDER_EXEC
};

enum class JobType : uint8_t {
    ADD,
    CANCEL,
    MODIFY
};



struct Order : ILLHook { //NOTE: Even if this is rearranged, there will still be 6 bytes of padding
    //This ends up being 88 bytes wide
    /*
    TODO: Get rid of 8 bytes by removing price from the order structure. Also remove
    user_id and create a separate map.
    1.) Remove price from struct => -8
    2.) Remove user_id and create separate map => -32
    3.) Replace type of ticker with char array of 8 bytes => -24
    4.) Add front/back pointer for intrusive linked list => +16
    5.) Replace u32 to u16 for quantity => -2
    6.) Remove order_id => -4
    
    */
    std::string ticker; //8
    std::string user_id;
    uint32_t order_id; //4
    OrderType order_type;// 1
    OrderSubType order_subtype;// 1
    double price; // 8
    uint32_t quantity; // 4    
};

struct FilledOrder { //Should be 56 bytes wide
    std::string ticker;
    std::string user_id;
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
    std::pair<double, std::list<Order>>* entry_list;
    std::list<Order>::iterator entry;
};

struct Event {
    EventType event_type;
    FilledOrder filled_order;
};

inline bool operator==(const Order &order_l, const Order &order_r) {
    return (
        order_l.ticker == order_r.ticker &&
        order_l.user_id == order_r.user_id &&
        order_l.order_type == order_r.order_type &&
        order_l.order_subtype == order_r.order_subtype &&
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
