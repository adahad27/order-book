#include "ledger.h"
/*
REQUIRES:
order must have been price matched to a resting order in book.

MODIFIES:
order.quantity, book[order.ticker][price]->queue.

EFFECT:
DOes price-time matching on the incoming order against resting orders in
opposite book. Will match most favorable trade until all quantity is exhausted,
before moving onto next most favorable trade.
*/
constexpr double TICK = 0.1;
constexpr double CENTER = 300;
constexpr double RANGE = 600;

void Ledger::resolve_order(auto& book, Order& order) {
    while (order.quantity > 0) {
        /*
        If queue.empty() -> we remove this price level from the map

        If outstanding >= queue.front().quantity -> pop item from queue,
        outstanding -= queue.front().quantity If outstanding <
        queue.front().quantity -> modify queue.front().quantity, outstanding = 0
        |
        +-> Add to record history if order is executed
        */
        double price = book[order.ticker].begin()->first;
        List<Order>& order_queue = book[order.ticker].begin()->second;

        FilledOrder filled_order;
        filled_order.ticker = order.ticker;
        filled_order.user_id = order.user_id;

        // TODO: Maybe try replacing this with template function?
        if (order.order_type == OrderType::BID) {
            filled_order.bidder_id = order.user_id;
            filled_order.asker_id = order_queue.front().user_id;
            filled_order.bid_order_id = order.order_id;
            filled_order.ask_order_id = order_queue.front().order_id;
        } else {
            filled_order.asker_id = order.user_id;
            filled_order.bidder_id = order_queue.front().user_id;
            filled_order.ask_order_id = order.order_id;
            filled_order.bid_order_id = order_queue.front().order_id;
        }
        filled_order.price = order_queue.front().price;
        filled_order.quantity =
            std::min(order_queue.front().quantity, order.quantity);

        Event matched_order = Event{.event_type = EventType::ORDER_EXEC,
                                    .filled_order = filled_order};
        event_history.emplace_back(matched_order);
        if (order.quantity >= order_queue.front().quantity) {
            order.quantity -= order_queue.front().quantity;
            outstanding_orders.erase(order_queue.front().order_id);
            order_queue.pop_front();

        } else {
            order_queue.front().quantity -= order.quantity;
            order.quantity = 0;
        }

        if (order_queue.empty()) {
            book[order.ticker].erase(price);
            return;
        }
    }
}

uint32_t Ledger::add_order_id(Order order, std::optional<uint32_t> order_id) {
    std::string& ticker{order.ticker};
    double price{order.price};
    uint32_t id = order_id.has_value() ? order_id.value() : (global_order_id++);

    order.order_id = id;

    auto execute_order = [&](auto& home_book, auto& counter_book, SortType sort,
                             std::function<bool(double, double)> compare) {
        if (order.order_subtype == OrderSubType::MARKET) {
            // If market order, then don't need to compare order price against
            // resting liquidity price
            compare = [](double a, double b) { return true; };
        }
        while (counter_book.contains(ticker) && !counter_book[ticker].empty() &&
               compare(price, counter_book[ticker].begin()->first)) {
            resolve_order(counter_book, order);
            if (order.quantity == 0) {
                break;
            }
        }
        if (order.quantity > 0) {
            if (!home_book.contains(ticker)) {
                home_book[ticker].init_map(TICK, CENTER, RANGE, sort);
            }
            home_book[ticker][price].push_back(order);
            outstanding_orders[id] =
                OrderEntry{home_book[ticker].find(price),
                           home_book[ticker][price].last()};
        }
    };

    if (order.order_type == OrderType::ASK) {
        execute_order(ask_book, bid_book, SortType::DESCENDING,
                      [](double a, double b) { return a <= b; });
    } else {
        execute_order(bid_book, ask_book, SortType::ASCENDING,
                      [](double a, double b) { return a >= b; });
    }

    if(!order_id.has_value()) {
        m_resp_queue.push(Response{.job_type = JobType::ADD, .order_id = id, .response = id});
    }
    return id;
}

uint32_t Ledger::add_order(Order order) {
    return add_order_id(order, std::optional<uint32_t>{});
}

void Ledger::cancel_order_helper(uint32_t order_id) {
    Order& order = *(outstanding_orders[order_id].entry);

    if (outstanding_orders[order_id].entry_list->second.size() == 1) {
        if (order.order_type == OrderType::ASK) {
            ask_book[order.ticker].erase(order.price);
        } else {
            bid_book[order.ticker].erase(order.price);
        }
    } else {
        outstanding_orders[order_id].entry_list->second.erase(
            outstanding_orders[order_id].entry);
    }
    // cannot do a lookup by price to find the queue to delete in because that
    // is O(logn)

    outstanding_orders.erase(order_id);
}

bool Ledger::cancel_order(uint32_t order_id) {
    if (!outstanding_orders.contains(order_id)) {
        m_resp_queue.push(Response{.job_type = JobType::ADD, .order_id = order_id, .response = false});
        return false;
    }

    cancel_order_helper(order_id);
    m_resp_queue.push(Response{.job_type = JobType::ADD, .order_id = order_id, .response = true});

    return true;
}

bool Ledger::modify_order(uint32_t order_id, Order order) {
    if (!outstanding_orders.contains(order_id)) {
        m_resp_queue.push(Response{.job_type = JobType::ADD, .order_id = order_id, .response = false});
        return false;
    }

    /*
    If price is different, then we must erase the intial value from
    the price level.
    Then we must insert a new order with the modified price into the
    order book.
    */
    outstanding_orders[order_id].entry->quantity = order.quantity;
    if (order.price != outstanding_orders[order_id].entry->price) {
        cancel_order(order_id);
        add_order_id(order, order_id);
    }
    m_resp_queue.push(Response{.job_type = JobType::ADD, .order_id = order_id, .response = true});

    return true;
}

void Ledger::print_events() {
    for (uint64_t i = 0; i < event_history.size(); ++i) {
        std::cout << "Event " << i << ": " << event_history[i];
    }
}

void Ledger::start_loop() {
    while (true) {
        /*
        Busy waiting is better than sleeping the CPU here because
        matching should be as responsive as possible even if it
        burns CPU cycles.
        */
        auto wrapped_job = m_req_queue.pop();
        if (!wrapped_job.has_value()) continue;
        Job job = wrapped_job.value();

        switch (job.job_type) {
            case JobType::ADD:
                add_order(job.order);
                break;
            case JobType::CANCEL:
                cancel_order(job.order_id);
                break;
            case JobType::MODIFY:
                modify_order(job.order_id, job.order);
                break;
        }
    }
}

void Ledger::start_ledger() {
    boost::thread engine_thread{&Ledger::start_loop, this};
}

const std::optional<Order> Ledger::get_best_ask(const std::string& ticker)  {
    if(!ask_book.contains(ticker) || !ask_book[ticker].begin() || ask_book[ticker].begin()->second.empty()) {
        return std::nullopt;
    }
    return ask_book[ticker].begin()->second.front();
    
}

const std::optional<Order> Ledger::get_best_bid(const std::string& ticker)  {
    if(!bid_book.contains(ticker) || !bid_book[ticker].begin() || bid_book[ticker].begin()->second.empty()) {
        return std::nullopt;
    }
    return bid_book[ticker].begin()->second.front();
}

const std::optional<double> Ledger::get_spread(const std::string& ticker)  {
    if( !ask_book.contains(ticker) || !ask_book[ticker].begin() || ask_book[ticker].begin()->second.empty() ||
        !bid_book.contains(ticker) || !bid_book[ticker].begin() || bid_book[ticker].begin()->second.empty()) {
        return std::nullopt;
    }

    return ask_book[ticker].begin()->first - bid_book[ticker].begin()->first;
}