#include "tester.h"
#include "ledger.h"
#include <cassert>
#include <iostream>
/*
How does the Tester work?
The Tester class has access to all of the methods and variables for the Ledger.
So given it has it's own Ledger, all of it's variables can be accessed through here.

Test only single-threaded first for basic functionality:
General:
    1.) After the best price level is completely matched, it is removed, and the next price level becomes the best

Add:
    1.) Adding to empty Order Book adds the order to the correct price and queue has size 1
    2.) Adding to non-empty Order Book at a different price allocates another price level
    3.) Adding to non-empty Order Book at same price enqueues behind first allocated Order

Matching:
    Complete Fill:
        1.) Two trades at the same exact price and the same exact quantity clear for Limit Orders
        2.) One Market Order and one Resting Order at the same exact price and same exact quantity clear
        3.) Two trades that are crossed at different prices with the same exact quantity clear for Limit Orders
        ->  Verify that it clears for the price of the Resting Order

        4.) One Market Order and one Resting Order that are crossed at different prices with the same exact quantity clear
        ->  Verify that it clears for the price of the Resting Order

    Partial Fill:
        1.) Incoming order has higher quantity than Resting order, but all orders apart from the Resting Order violate price
        ->  Verify that only the Resting order's quantity is cleared, and part of the Incoming order is passed as a Resting Order

        2.) Incoming order has higher quantity than Resting order, but there exists an order apart from Resting order that does not violate price
        ->  Verity that the Resting order's quantity is cleared, and then the rest of the Incoming order goes towards the next favorable order

        3.) Incoming order has lower quantity than Resting order
        ->  Verify that the Incoming order's quantity is cleared, but the Resting order still remains on the book with decreased quantity

Cancellation:
    1.) Cancelling the only order remove the entire price level
    2.) Cancel the head of a price level
    3.) Cancel in the middle of a price level
    4.) Cancel at the tail of a price level
    5.) Cancel non-existent ID
    6.) Double Cancellation
    7.) Cancelling an order after it has been executed
    8.) Cancel a partially filled order

Modification:
    1.) Quantity modification does not change price level
    2.) Price increase changes price level
    3.) Price decrease changes price level
    4.) Modified to cross spread then it should execcute immediately


Given all of these testcases, then the tester should be able to assert on the condition of the bid/ask books.
*/

static bool order_equal(const Order &lhs, const Order &rhs) {
    return lhs.ticker == rhs.ticker &&
           lhs.user_id == rhs.user_id &&
           lhs.order_type == rhs.order_type &&
           lhs.order_subtype == rhs.order_subtype &&
           lhs.price == rhs.price &&
           lhs.quantity == rhs.quantity;
}

int main() {
    WaitQueue<Job> req_queue;
    WaitQueue<uint32_t> resp_queue;

    Ledger book(req_queue, resp_queue);
    Tester tester(book);

    std::cout << "Testing Add Orders...\n";
    {
        Order sell = {
            .ticker = "GOOGL",
            .user_id = "User 1",
            .order_type = OrderType::ASK,
            .order_subtype = OrderSubType::LIMIT,
            .price = 101,
            .quantity = 5,
        };
        tester.add_order(sell);
        assert(tester.ask_book.count("GOOGL") == 1);
        assert(tester.ask_book.at("GOOGL").size() == 1);
        assert(tester.ask_book.at("GOOGL").count(101) == 1);
        assert(tester.ask_book.at("GOOGL").at(101).size() == 1);
        assert(order_equal(tester.ask_book.at("GOOGL").at(101).front(), sell));

        Order sell2 = {
            .ticker = "GOOGL",
            .user_id = "User 2",
            .order_type = OrderType::ASK,
            .order_subtype = OrderSubType::LIMIT,
            .price = 100,
            .quantity = 5,
        };
        tester.add_order(sell2);
        assert(tester.ask_book.at("GOOGL").size() == 2);
        assert(tester.ask_book.at("GOOGL").count(100) == 1);
        assert(tester.ask_book.at("GOOGL").at(100).size() == 1);
        assert(order_equal(tester.ask_book.at("GOOGL").at(100).front(), sell2));

        Order sell3 = {
            .ticker = "GOOGL",
            .user_id = "User 3",
            .order_type = OrderType::ASK,
            .order_subtype = OrderSubType::LIMIT,
            .price = 100,
            .quantity = 7,
        };
        tester.add_order(sell3);
        assert(tester.ask_book.at("GOOGL").at(100).size() == 2);
        assert(order_equal(tester.ask_book.at("GOOGL").at(100).front(), sell2));
        assert(order_equal(tester.ask_book.at("GOOGL").at(100).back(), sell3));
    }
    std::cout << "All Add Orders passed...\n";

    std::cout << "Testing Matching...\n";
    {
        Order resting = {
            .ticker = "AMZN",
            .user_id = "Maker",
            .order_type = OrderType::ASK,
            .order_subtype = OrderSubType::LIMIT,
            .price = 100,
            .quantity = 5,
        };
        tester.add_order(resting);
        Order incoming = {
            .ticker = "AMZN",
            .user_id = "Taker",
            .order_type = OrderType::BID,
            .order_subtype = OrderSubType::LIMIT,
            .price = 100,
            .quantity = 5,
        };
        tester.add_order(incoming);
        assert(tester.ask_book.at("AMZN").empty());
        assert(tester.bid_book.at("AMZN").empty());
    }

    {
        Order resting = {
            .ticker = "AAPL",
            .user_id = "Maker",
            .order_type = OrderType::ASK,
            .order_subtype = OrderSubType::LIMIT,
            .price = 105,
            .quantity = 5,
        };
        tester.add_order(resting);
        Order incoming = {
            .ticker = "AAPL",
            .user_id = "Taker",
            .order_type = OrderType::BID,
            .order_subtype = OrderSubType::MARKET,
            .price = 0,
            .quantity = 5,
        };
        tester.add_order(incoming);
        assert(tester.ask_book.at("AAPL").empty());
        assert(tester.bid_book.at("AAPL").empty());
    }

    {
        Order resting = {
            .ticker = "MSFT",
            .user_id = "Maker",
            .order_type = OrderType::ASK,
            .order_subtype = OrderSubType::LIMIT,
            .price = 110,
            .quantity = 5,
        };
        tester.add_order(resting);
        Order incoming = {
            .ticker = "MSFT",
            .user_id = "Taker",
            .order_type = OrderType::BID,
            .order_subtype = OrderSubType::LIMIT,
            .price = 115,
            .quantity = 5,
        };
        tester.add_order(incoming);
        assert(tester.ask_book.at("MSFT").empty());
        assert(tester.bid_book.at("MSFT").empty());
    }

    {
        Order resting = {
            .ticker = "NFLX",
            .user_id = "Maker",
            .order_type = OrderType::ASK,
            .order_subtype = OrderSubType::LIMIT,
            .price = 120,
            .quantity = 5,
        };
        tester.add_order(resting);
        Order incoming = {
            .ticker = "NFLX",
            .user_id = "Taker",
            .order_type = OrderType::BID,
            .order_subtype = OrderSubType::MARKET,
            .price = 0,
            .quantity = 5,
        };
        tester.add_order(incoming);
        assert(tester.ask_book.at("NFLX").empty());
        assert(tester.bid_book.at("NFLX").empty());
    }

    {
        Order ask1 = {
            .ticker = "FB",
            .user_id = "Maker1",
            .order_type = OrderType::ASK,
            .order_subtype = OrderSubType::LIMIT,
            .price = 130,
            .quantity = 5,
        };
        Order ask2 = {
            .ticker = "FB",
            .user_id = "Maker2",
            .order_type = OrderType::ASK,
            .order_subtype = OrderSubType::LIMIT,
            .price = 140,
            .quantity = 5,
        };
        tester.add_order(ask1);
        tester.add_order(ask2);
        Order incoming = {
            .ticker = "FB",
            .user_id = "Taker",
            .order_type = OrderType::BID,
            .order_subtype = OrderSubType::LIMIT,
            .price = 135,
            .quantity = 8,
        };
        tester.add_order(incoming);
        assert(tester.ask_book.at("FB").size() == 1);
        assert(tester.ask_book.at("FB").count(140) == 1);
        assert(tester.ask_book.at("FB").at(140).front().quantity == 5);
        assert(tester.bid_book.at("FB").count(135) == 1);
        assert(tester.bid_book.at("FB").at(135).front().quantity == 3);
    }

    {
        Order ask1 = {
            .ticker = "IBM",
            .user_id = "Maker1",
            .order_type = OrderType::ASK,
            .order_subtype = OrderSubType::LIMIT,
            .price = 130,
            .quantity = 5,
        };
        Order ask2 = {
            .ticker = "IBM",
            .user_id = "Maker2",
            .order_type = OrderType::ASK,
            .order_subtype = OrderSubType::LIMIT,
            .price = 132,
            .quantity = 5,
        };
        tester.add_order(ask1);
        tester.add_order(ask2);
        Order incoming = {
            .ticker = "IBM",
            .user_id = "Taker",
            .order_type = OrderType::BID,
            .order_subtype = OrderSubType::LIMIT,
            .price = 135,
            .quantity = 8,
        };
        tester.add_order(incoming);
        assert(tester.ask_book.at("IBM").size() == 1);
        assert(tester.ask_book.at("IBM").count(132) == 1);
        assert(tester.ask_book.at("IBM").at(132).front().quantity == 2);
        assert(tester.bid_book.at("IBM").empty());
    }

    {
        Order ask = {
            .ticker = "TSLA",
            .user_id = "Maker",
            .order_type = OrderType::ASK,
            .order_subtype = OrderSubType::LIMIT,
            .price = 140,
            .quantity = 10,
        };
        tester.add_order(ask);
        Order incoming = {
            .ticker = "TSLA",
            .user_id = "Taker",
            .order_type = OrderType::BID,
            .order_subtype = OrderSubType::LIMIT,
            .price = 140,
            .quantity = 4,
        };
        tester.add_order(incoming);
        assert(tester.ask_book.at("TSLA").count(140) == 1);
        assert(tester.ask_book.at("TSLA").at(140).front().quantity == 6);
        assert(tester.bid_book.at("TSLA").empty());
    }
    std::cout << "All Matching tests passed...\n";

    std::cout << "Testing Cancel Orders...\n";
    {
        Order ask = {
            .ticker = "ORCL",
            .user_id = "User1",
            .order_type = OrderType::ASK,
            .order_subtype = OrderSubType::LIMIT,
            .price = 150,
            .quantity = 5,
        };
        uint32_t id = tester.add_order(ask);
        assert(tester.cancel_order(id));
        assert(tester.ask_book.at("ORCL").empty());
    }

    {
        Order ask1 = {
            .ticker = "SAP",
            .user_id = "User1",
            .order_type = OrderType::ASK,
            .order_subtype = OrderSubType::LIMIT,
            .price = 150,
            .quantity = 5,
        };
        Order ask2 = {
            .ticker = "SAP",
            .user_id = "User2",
            .order_type = OrderType::ASK,
            .order_subtype = OrderSubType::LIMIT,
            .price = 150,
            .quantity = 5,
        };
        Order ask3 = {
            .ticker = "SAP",
            .user_id = "User3",
            .order_type = OrderType::ASK,
            .order_subtype = OrderSubType::LIMIT,
            .price = 150,
            .quantity = 5,
        };
        uint32_t id1 = tester.add_order(ask1);
        uint32_t id2 = tester.add_order(ask2);
        uint32_t id3 = tester.add_order(ask3);
        assert(tester.ask_book.at("SAP").at(150).size() == 3);

        assert(tester.cancel_order(id2));
        assert(tester.ask_book.at("SAP").at(150).size() == 2);
        assert(order_equal(tester.ask_book.at("SAP").at(150).front(), ask1));
        assert(order_equal(tester.ask_book.at("SAP").at(150).back(), ask3));

        assert(tester.cancel_order(id1));
        assert(tester.ask_book.at("SAP").at(150).size() == 1);
        assert(order_equal(tester.ask_book.at("SAP").at(150).front(), ask3));

        assert(tester.cancel_order(id3));
        assert(tester.ask_book.at("SAP").empty());
    }

    {
        Order ask = {
            .ticker = "IBM2",
            .user_id = "UserX",
            .order_type = OrderType::ASK,
            .order_subtype = OrderSubType::LIMIT,
            .price = 160,
            .quantity = 5,
        };
        uint32_t id = tester.add_order(ask);
        assert(!tester.cancel_order(id + 1000));
        assert(tester.cancel_order(id));
        assert(!tester.cancel_order(id));
    }

    {
        Order ask = {
            .ticker = "VZ",
            .user_id = "Maker",
            .order_type = OrderType::ASK,
            .order_subtype = OrderSubType::LIMIT,
            .price = 170,
            .quantity = 5,
        };
        tester.add_order(ask);
        Order bid = {
            .ticker = "VZ",
            .user_id = "Taker",
            .order_type = OrderType::BID,
            .order_subtype = OrderSubType::LIMIT,
            .price = 170,
            .quantity = 5,
        };
        uint32_t bid_id = tester.add_order(bid);
        assert(tester.ask_book.at("VZ").empty());
        assert(tester.bid_book.at("VZ").empty());
        assert(!tester.cancel_order(bid_id));
    }

    {
        Order ask = {
            .ticker = "BABA",
            .user_id = "Maker",
            .order_type = OrderType::ASK,
            .order_subtype = OrderSubType::LIMIT,
            .price = 180,
            .quantity = 10,
        };
        uint32_t ask_id = tester.add_order(ask);
        Order bid = {
            .ticker = "BABA",
            .user_id = "Taker",
            .order_type = OrderType::BID,
            .order_subtype = OrderSubType::LIMIT,
            .price = 180,
            .quantity = 5,
        };
        tester.add_order(bid);
        assert(tester.ask_book.at("BABA").count(180) == 1);
        assert(tester.ask_book.at("BABA").at(180).front().quantity == 5);
        assert(tester.cancel_order(ask_id));
        assert(tester.ask_book.at("BABA").empty());
    }
    std::cout << "All Cancel Orders passed...\n";

    std::cout << "Testing Modify Orders...\n";
    {
        Order ask = {
            .ticker = "ZM",
            .user_id = "Maker",
            .order_type = OrderType::ASK,
            .order_subtype = OrderSubType::LIMIT,
            .price = 190,
            .quantity = 10,
        };
        uint32_t ask_id = tester.add_order(ask);
        Order modified = ask;
        modified.quantity = 5;
        assert(tester.modify_order(ask_id, modified));
        assert(tester.ask_book.at("ZM").count(190) == 1);
        assert(tester.ask_book.at("ZM").at(190).front().quantity == 5);
    }

    {
        Order ask = {
            .ticker = "TWTR",
            .user_id = "Maker",
            .order_type = OrderType::ASK,
            .order_subtype = OrderSubType::LIMIT,
            .price = 195,
            .quantity = 5,
        };
        uint32_t ask_id = tester.add_order(ask);
        Order modified = ask;
        modified.price = 205;
        assert(tester.modify_order(ask_id, modified));
        assert(tester.ask_book.at("TWTR").count(205) == 1);
        assert(tester.ask_book.at("TWTR").at(205).front().quantity == 5);
    }

    {
        Order ask = {
            .ticker = "UBER",
            .user_id = "Maker",
            .order_type = OrderType::ASK,
            .order_subtype = OrderSubType::LIMIT,
            .price = 210,
            .quantity = 5,
        };
        uint32_t ask_id = tester.add_order(ask);
        Order modified = ask;
        modified.price = 205;
        assert(tester.modify_order(ask_id, modified));
        assert(tester.ask_book.at("UBER").count(205) == 1);
        assert(tester.ask_book.at("UBER").at(205).front().quantity == 5);
    }

    {
        Order bid = {
            .ticker = "LYFT",
            .user_id = "BidMaker",
            .order_type = OrderType::BID,
            .order_subtype = OrderSubType::LIMIT,
            .price = 220,
            .quantity = 5,
        };
        tester.add_order(bid);
        Order ask = {
            .ticker = "LYFT",
            .user_id = "AskMaker",
            .order_type = OrderType::ASK,
            .order_subtype = OrderSubType::LIMIT,
            .price = 230,
            .quantity = 5,
        };
        uint32_t ask_id = tester.add_order(ask);
        Order modified = ask;
        modified.price = 215;
        assert(tester.modify_order(ask_id, modified));
        assert(tester.ask_book.at("LYFT").empty());
        assert(tester.bid_book.at("LYFT").empty());
    }

    std::cout << "All Modify Orders passed...\n";
    return 0;
}
