## Brief Summary and Future Plans
This project was started in May 2026. The intent of this project is to build a order-book that can replicate an exchange order-book given incoming order-flow. This project assumes that the underlying security is a regular stock that can be represented with a ticker symbol. This project will also be tested using a NASDAQ-ITCH sample dataset. As of right now, this dataset will only be used to test the correctness of the system and not concurrency. The dataset can be downloaded from [here](https://emi.nasdaq.com/ITCH/Nasdaq%20ITCH/01302019.NASDAQ_ITCH50.gz). The dataset was also isolated specifically for the AAPL ticker using a python script.

### Supported Items and Implementation Specific Details:
1. Support for Limit Orders, Market Orders
2. Order cancellation/modification
3. Matching Engine uses Price-Time matching
4. Event-Driven server architecture
5. Memory Pools
6. Lock Free Single Producer Single Consumer Queue

### Planned Items and Implementation Specific Details:
1. Support for Stop-Loss, FillOrKill, and ImmediateOrCancel orders
2. Memory Optimization tricks like Intrusive Linked Lists
3. Add persistence logging in case of machine-crashes
4. Migrating Price Levels from using Sorted Trees to using 

## Architecture Description:
There are two major components in this project:
1. Server: Responsible for interfacing with clients over TCP
2. Core Engine: Responsible for executing client orders

### Server
The server is single-threaded, and it uses the event loop paradigm to serve client requests. The server will also be responsible for creating two queues per ticker, one for requests and one for responses. Upon receiving a request for an order from the client, the server will parse the request for correctness, and then pass it to the request queue, and then proceed to block on the response queue waiting for the order book to confirm/deny the validity of the order. The response result from
the response queue will then be forwarded back to the user.

### Core Engine (TODO)
The Core Engine will be multi-threaded. There will be one thread per ticker, because there is no shared state in the ask or bid books across tickers. So each request-response queue per ticker will have only one producer and one consumer.

## Design Decisions and Justification

**Question 1:** Why use event-looping over spawning a new thread per client connection?

**Answer:** Event-looping was used because each connection should be relatively short-lived considering that the server should respond immediately after confirming that the order has been received. The work of inserting into the Order Book itself need not be involved to confirm order receival.


**Question 2:** Why use message passing over conventional mutexes and locking?

**Answer:** Message passing was used over conventional shared memory because I think it leads to cleaner inter-thread communication when communication is made explict as opposed to reading shared memory which is implicit. It also prevents issues with unlocking at the wrong moment, since threads are completely separate, there is no worry about having to "unlock". This way everything can be written like it was single-threaded code which is much easier to reason about.


**Question 3:** Why use a memory pool?

**Answer:** Heap allocation is slow and so we want to avoid heap allocation in our hot path if possible, specifically when malloc/free must be called many times. There are two types of heap allocation that were not reduced in this project:

1. When an unseen ticker has an order sent for it, at that point, a new book specifically for that ticker must be produced, and at the time of writing, I am assuming that the number of tickers is unknown therefore this cost must be paid. A production system would know how many tickers exist in the market and what each ticker is, so this could also be turned into a one-time heap allocation as opposed to having to allocate books per ticker dynamically.

2. The startup cost of actually allocating the pool must be paid. Since this memory can't be stack-allocated, the only two other reasonable places to do it would be in the static section of memory and the heap. I wanted to leave the pool flexible in how many objects can be allocated, so I opted to create the pool on the heap.


**Question 4:** Did you benchmark a regular queue protected by a mutex before deciding to implement an SPSC queue?

**Answer:** No I did not. I think that the general engineering advice of turning towards a more complicated solution only if the simpler solution is lacking is true, however I wanted to implement an SPSC queue for the learning experience of being able to write code that exploits atomics and memory ordering. I do have plans in the future of measuring the latency of each, but that would be when directly testing the order book against something that simulates market data. At the time of implementation of the SPSC queue, the infrastructure of testing the order book against simulated market data had not been implemented.