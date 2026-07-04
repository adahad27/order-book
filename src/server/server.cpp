#include "ledger.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <poll.h>
#include <vector>
#include <fcntl.h>
#include <iostream>

Ledger book;

const uint32_t port = 8080;
const uint32_t MAX_MSG_LEN = 1000;

struct Connection_State {

    int fd = -1;
    
    bool want_read = false;
    bool want_write = false;
    bool want_close = false;

    std::vector<uint8_t> incoming;
    std::vector<uint8_t> outgoing;
};



void append_response(Connection_State &state, const std::string &response) {
    state.outgoing.insert(state.outgoing.end(), response.begin(), response.end());
}

bool parse_order_type(const std::string &value, OrderType &order_type) {
    if(value == "BID" || value == "bid") {
        order_type = OrderType::BID;
        return true;
    }
    if(value == "ASK" || value == "ask") {
        order_type = OrderType::ASK;
        return true;
    }
    return false;
}

bool parse_order_subtype(const std::string &value, OrderSubType &order_subtype) {
    if(value == "MARKET" || value == "market") {
        order_subtype = OrderSubType::MARKET;
        return true;
    }
    if(value == "LIMIT" || value == "limit") {
        order_subtype = OrderSubType::LIMIT;
        return true;
    }
    if(value == "STOP_LOSS" || value == "stop_loss") {
        order_subtype = OrderSubType::STOP_LOSS;
        return true;
    }
    if(value == "FILL_OR_KILL" || value == "fill_or_kill") {
        order_subtype = OrderSubType::FILL_OR_KILL;
        return true;
    }
    if(value == "IMMED_OR_CANCEL" || value == "immed_or_cancel") {
        order_subtype = OrderSubType::IMMED_OR_CANCEL;
        return true;
    }
    return false;
}

bool build_order(const std::vector<std::string> &arguments, size_t offset, Order &order) {
    if(arguments.size() < offset + 6) {
        return false;
    }

    order.ticker = arguments[offset + 0];
    order.user_id = arguments[offset + 1];
    if(!parse_order_type(arguments[offset + 2], order.order_type)) {
        return false;
    }
    if(!parse_order_subtype(arguments[offset + 3], order.order_subtype)) {
        return false;
    }
    try {
        order.price = std::stod(arguments[offset + 4]);
        order.quantity = static_cast<uint32_t>(std::stoul(arguments[offset + 5]));
    } catch(...) {
        return false;
    }
    return true;
}

void handle_write(Connection_State &state) {
    //Write contents of outgoing to file descriptor here.
    size_t msg_len = state.outgoing.size();

    if(send(state.fd, &msg_len, 4, MSG_NOSIGNAL) <= 0) {
        state.want_close = true;
        state.want_write = false;
        return;
    }

    if(send(state.fd, state.outgoing.data(), msg_len, MSG_NOSIGNAL) <= 0) {
        state.want_close = true;
        state.want_write = false;
        return;
    }

    state.want_close = true;
    state.want_write = false;
}

void process_request(Connection_State &state) {
    int num_cmds;
    int arg_len;

    std::vector<std::string> arguments;

    memcpy(&num_cmds, state.incoming.data(), 4);
    arguments.resize(num_cmds);

    uint32_t buffer_end = 4;

    for(int i = 0; i < num_cmds; ++i) {
        memcpy(&arg_len, state.incoming.data() + buffer_end, 4);
        buffer_end += 4;
        for(int j = 0; j < arg_len; ++j) {
            arguments[i].push_back(state.incoming[buffer_end + j]);
        }
        buffer_end += arg_len;
    }

    if(arguments.empty()) {
        append_response(state, "ERROR: empty request");
        return;
    }

    const std::string &command = arguments[0];

    if(command == "add") {
        Order order;
        if(!build_order(arguments, 1, order)) {
            append_response(state, "ERROR: invalid add arguments\n");
            return;
        }
        uint32_t order_id = book.add_order(order);
        append_response(state, "ORDER_ADDED:" + std::to_string(order_id));
    }
    else if(command == "cancel") {
        if(arguments.size() != 2) {
            append_response(state, "ERROR: cancel requires order_id\n");
            return;
        }
        uint32_t order_id = 0;
        try {
            order_id = static_cast<uint32_t>(std::stoul(arguments[1]));
        } catch(...) {
            append_response(state, "ERROR: invalid order_id\n");
            return;
        }
        bool ok = book.cancel_order(order_id);
        append_response(state, ok ? "ORDER_CANCELLED" : "ORDER_NOT_FOUND");
    }
    else if(command == "modify") {
        if(arguments.size() != 8) {
            append_response(state, "ERROR: modify requires order_id and full order fields\n");
            return;
        }
        uint32_t order_id = 0;
        try {
            order_id = static_cast<uint32_t>(std::stoul(arguments[1]));
        } catch(...) {
            append_response(state, "ERROR: invalid order_id\n");
            return;
        }
        Order order;
        if(!build_order(arguments, 2, order)) {
            append_response(state, "ERROR: invalid modify arguments\n");
            return;
        }
        bool ok = book.modify_order(order_id, order);
        append_response(state, ok ? "ORDER_MODIFIED" : "ORDER_NOT_FOUND");
    }
    else {
        append_response(state, "ERROR: unknown command\n");
    }
}

int handle_read(Connection_State &state) {

    uint32_t cmds, len;

    if(recv(state.fd, (char*)&cmds, 4, 0) <= 0) {
        state.want_close = true;
        return -1;
    }
    state.incoming.resize(4);
    memcpy(state.incoming.data(), &cmds, 4);

    for(uint32_t i = 0; i < cmds; ++i) {

        uint32_t buffer_end = state.incoming.size();
        state.incoming.resize(buffer_end + 4);

        if(recv(state.fd, state.incoming.data() + buffer_end, 4, 0) <= 0) {
            state.want_close = true;
            return -1;
        }

        memcpy(&len, state.incoming.data() + buffer_end, 4);
        buffer_end = state.incoming.size();
        state.incoming.resize(buffer_end + len);
        
        if(recv(state.fd, state.incoming.data() + buffer_end, len, 0) <= 0) {
            state.want_close = true;
            return -1;
        }

    }
    
    state.want_read = false;
    process_request(state);
    handle_write(state);

    return 0;
}

void add_connection(int incoming_fd, std::vector<pollfd> &connections, std::vector<Connection_State> &states) {
    
    pollfd pfd;
    pfd.fd = incoming_fd;
    pfd.events = 0;
    pfd.revents = 0;


    Connection_State state;
    state.fd = incoming_fd;
    state.want_read = true;

    connections.push_back(pfd);
    states.push_back(state);
}

void delete_connection(int idx, std::vector<pollfd> &connections, std::vector<Connection_State> &states) {

    connections[idx] = connections[connections.size() - 1];
    states[idx] = states[states.size() - 1];

    connections.pop_back();
    states.pop_back();

}

int init_server() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(fd, F_SETFL, O_NONBLOCK);

    if(fd < 0) {
        std::cout << "Server does not have enough open file descriptors\n";
    }

    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &val, sizeof(val));

    sockaddr_in addr{};

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);


    bind(fd, (struct sockaddr*)&addr, sizeof(addr));
    return fd;
}

void run_server(int fd) {
    listen(fd, MAX_MSG_LEN);

    struct sockaddr_in client{};
    socklen_t addrlen = sizeof(client);



    std::vector<pollfd> connections;
    std::vector<Connection_State> states;

    add_connection(fd, connections, states);
    

    while(true) {

        for(uint32_t i = 0; i < connections.size(); ++i) {
            connections[i].events = 0;
            if(states[i].want_read) {
                connections[i].events |= POLLIN;
            }
            if(states[i].want_write) {
                connections[i].events |= POLLOUT;
            }
            if(states[i].want_close) {
                connections[i].events |= POLLHUP;
            }
        }

        poll(connections.data(), connections.size(), -1);


        for(uint32_t i = 0; i < connections.size(); ++i) {
            if(connections[i].revents & POLLIN) {

                if(connections[i].fd == fd) {
                    int incoming_fd = accept(fd, (sockaddr*) &client, &addrlen);
                    add_connection(incoming_fd, connections, states);
                    connections[i].revents = 0;
                    std::cout << "Connection established on socket: " << incoming_fd << std::endl;
                }
                else {
                    handle_read(states[i]);
                }
            }
            if(connections[i].revents & POLLOUT) {
                handle_write(states[i]);
            }
            if(connections[i].revents & POLLHUP || states[i].want_close) {
                int socket_fd = connections[i].fd;
                delete_connection(i, connections, states);
                close(socket_fd);
                std::cout << "Connection closed on socket: " << socket_fd << std::endl;
                // std::cout << "Current history is: " << std::endl;
                // book.print_events();
            }
        }

    }
}

int main() {

    int fd = init_server();

    run_server(fd);

    return 0;
}