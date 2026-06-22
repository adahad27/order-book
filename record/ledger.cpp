#include "ledger.h"


uint32_t Ledger::add_order(const std::string &ticker, const Order &order, double price) {

    if(order.order_type == OrderType::ASK) {
        ask_book[ticker][price].push_back(order);
    } else {
        bid_book[ticker][price].push_back(order);
    }

    return 0;
}