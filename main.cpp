#include "ledger.h"
int main() {
    Ledger book;

    // for (int i = 0; i < 10; i++) {
    //     book.add_order(Order{
    //         .ticker = "GOOGL",
    //         .user_id = "abhishek",
    //         .order_type = OrderType::ASK,
    //         .price = (double)i * 100,
    //         .quantity = 3,
    //     });
    // }

    // for (int i = 0; i < 10; i++) {
    //     book.add_order(Order{
    //         .ticker = "GOOGL",
    //         .user_id = "abhishek",
    //         .order_type = OrderType::BID,
    //         .price = (double)i * 100,
    //         .quantity = 3,
    //     });
    // }

    book.add_order(Order{
        .ticker = "GOOGL",
        .user_id = "User 1",
        .order_type = OrderType::ASK,
        .price = 101,
        .quantity = 5,
    });

    book.add_order(Order{
        .ticker = "GOOGL",
        .user_id = "User 2",
        .order_type = OrderType::ASK,
        .price = 102,
        .quantity = 8,
    });

    book.add_order(Order{
        .ticker = "GOOGL",
        .user_id = "User 3",
        .order_type = OrderType::BID,
        .price = 99,
        .quantity = 10,
    });

    book.add_order(Order{
        .ticker = "GOOGL",
        .user_id = "User 5",
        .order_type = OrderType::BID,
        .price = 99,
        .quantity = 10,
    });

    book.add_order(Order{
        .ticker = "GOOGL",
        .user_id = "User 6",
        .order_type = OrderType::BID,
        .price = 99,
        .quantity = 10,
    });

    //All are resting orders so far

    book.add_order(Order{
        .ticker = "GOOGL",
        .user_id = "User 4",
        .order_type = OrderType::BID,
        .price = 102,
        .quantity = 7,
    });

    book.add_order(Order{
        .ticker = "GOOGL",
        .user_id = "User 7",
        .order_type = OrderType::ASK,
        .price = 98,
        .quantity = 7,
    });

    book.print_events();

    return 0;
}