#include <sstream>

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

    // All are resting orders so far

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

    // std::string input;

    // std::vector<std::string> tokens;
    // std::string token;

    // while (true) {
    //     input.clear();
    //     tokens.clear();
    //     std::cout << "Waiting for next trade...\n";
    //     std::cin >> input;

    //     std::stringstream ss(input);
    //     while (std::getline(ss, token, ' ')) {
    //         tokens.push_back(token);
    //     }
    //     //<trade_type> <ticker> <quantity> <price>
    //     if (tokens.size() == 0) {
    //         std::cout << "No input was entered...\n";
    //         continue;
    //     } else if (tokens.size() < 4) {
    //         std::cout << "Did not enter complete order...\n";
    //         continue;
    //     } else if (tokens.size() > 4) {
    //         std::cout << "Entered too many arguments...\n";
    //         continue;
    //     }

    //     // Need to add way more error checking here
    //     Order new_order{
    //         .ticker = tokens[1],
    //         .user_id = "abhishek",
    //         .order_type = tokens[0] == "ASK" ? OrderType::ASK : OrderType::BID,
    //         .price = std::stod(tokens[3]),
    //         .quantity = (uint32_t) std::stoul(tokens[2])};
    // }

    return 0;
}