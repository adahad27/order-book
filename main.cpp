#include "ledger.h"
int main() {
    Ledger book;

    for (int i = 0; i < 10; i++) {
        book.add_order(Order{
            .ticker = "GOOGL",
            .user_id = "abhishek",
            .order_type = OrderType::ASK,
            .price = 30,
            .quantity = 3,
        });
    }

    for (int i = 0; i < 10; i++) {
        book.add_order(Order{
            .ticker = "GOOGL",
            .user_id = "abhishek",
            .order_type = OrderType::BID,
            .price = 30,
            .quantity = 3,
        });
    }

    return 0;
}