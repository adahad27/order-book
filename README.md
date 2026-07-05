## Brief Summary and Future Plans
This project was started in May 2026. The intent of this project is to build a order-book that can function as an exchange. This project assumes that the underlying security is a regular stock that can be represented with a ticker symbol.

### Supported Items:
1. Support for Limit Orders, Market Orders
2. Order cancellation/modification
3. Matching Engine uses Price-Time matching
4. Event-Driven server architecture

### Planned Items:
1. Lock Free Single Producer Single Consumer Queue
2. Support for Stop-Loss, FillOrKill, and ImmediateOrCancel orders
3. Memory Optimization tricks like Intrusive Linked Lists and Memory Pools
4. Add persistence logging in case of machine-crashes

## Architecture Description:
There are two major components in this project:
1. Server: Responsible for interfacing with clients over TCP
2. Core Engine: Responsible for executing client orders

### Server
The server is single-threaded, and it uses the event loop paradigm to serve client requests. The server will also be responsible for creating two queues per ticker, one for requests and one for responses. Upon receiving a request for an order from the client, the server will parse the request for correctness, and then pass it to the request queue, and then proceed to block on the response queue waiting for the order book to confirm/deny the validity of the order. The response result from
the response queue will then be forwarded back to the user.

### Core Engine (TODO)
The Core Engine will be multi-threaded. There will be one thread per ticker, because there is no shared state in the ask or bid books across tickers. So each request-response queue per ticker will have only one producer and one consumer.

