# Server Models Playground

This project is a personal playground with the goal of understanding the different server concurrency models.

The idea was to implement a small set of servers, each one with a different approach as how to serve clients concurrently. The code is mostly written in (a rudimentary version of) C as the goal here was to be closer to the OS as I could, while trying stuff out.


## Running the project

So the project includes a [client](./client.py) and multiple [server implementations](./server). To run it:

```sh
# compile and launch the server
cd server && make
./bin/02_preforked_processes # this directory will have all the compiled servers

# launch the client
python3 client.py 3 # launch 3 concurrent clients
```

Each one of the clients will send 5 messages to the server and terminate the connection (see the protocol information bellow).


## Client <-> Server protocol

The protocol between the client and server is as stupid/simple as I could imagine it:

1. All messages are expected to be ASCII encoded
1. A [End-of-Text character (ETX)](https://en.wikipedia.org/wiki/End-of-Text_character) signals the end of a message sent by the client -> server
1. Each message sent by the client is to be acknowledge by the server with the string `ack`
1. A client message containing only `goodbye` signals to the server that the connection can be closed


## Server implementations

### Single process 

The [01_single_process.c](./server/src/01_single_process.c) is a server implementation where a single process/thread is used. The server sleeps for 2 seconds before "ack"ing each message.

The goal here is to show that the server will only handle one client at a time.

----

### Prefork server

The [02_preforked_processes.c](./server/src/02_prefork_processes.c) is a server who forks itself 5 times, before starting to accept client connections.

Each worker process also waits 2 seconds before acknowledging each client message. As expected, up to 5 concurrent clients can be handled in parallel.

----

### Multi-threaded server

The [03_threaded.c](./server/src/03_threaded.c) is a single-process, but multi-threaded server. It launches 5 worker threads.

It has a very "vanilla" implementation, where the main thread accepts incoming connections, but then enqueues them in a queue datastructure which is shared between all threads (without reading data from the socket).

One of the worker threads then picks-up that connection and deals with the whole read -> response cycle with the client.

Once again, as expected up to concurrent 5 clients can be handled in parallel.

----

### epoll based server

The [04_epoll.c](./server/src/04_epoll.c) is a single-threaded server which uses Linux's [epoll API](https://www.man7.org/linux/man-pages/man7/epoll.7.html) to receive I/O notifications in regards to the socket.

It leverages the API to handle two type of events:

1. New client connections that are ready to be `accept`ed
1. Client connections from who some data is read to be `read`

The goal here was to get a grasp on what an "event loop" looks like and to factually see that a single thread was able to handle multiple clients concurrently.



## Useful Reading

Random sources I've used throughout the project:

* [The method to epolls madness](https://medium.com/@copyconstruct/the-method-to-epolls-madness-d9d2d6378642)
* [Unicorn webserver design info](https://yhbt.net/unicorn/DESIGN.html)
* [Unicorn UNIX magic tricks](https://thorstenball.com/blog/2014/11/20/unicorn-unix-magic-tricks/)
* [Fun epoll quirks](https://medium.com/@copyconstruct/the-method-to-epolls-madness-d9d2d6378642)
* [Epoll is fundamentally broken](https://idea.popcount.org/2017-02-20-epoll-is-fundamentally-broken-12/)
* [Julia's awesome explanation around epoll + accept](https://jvns.ca/blog/2017/06/03/async-io-on-linux--select--poll--and-epoll/)
* [So much gold around all these patterns](https://luminousmen.com/post/asynchronous-programming-blocking-and-non-blocking)
