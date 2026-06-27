# Multithreaded TCP Message Broker

A C++17 TCP message broker built with Boost.Asio. Multiple clients can connect over TCP, subscribe to topics, publish messages, and receive messages through a simple pub-sub protocol.

This project is designed as a backend/networking learning project and demonstrates sockets, per-client concurrency, pub-sub routing, message queues, logging, metrics, idle timeouts, and unit-style tests.

## Features

- TCP server using Boost.Asio
- Multiple concurrent clients
- Fixed-size worker thread pool for client sessions
- Pub-sub topics/channels
- Thread-safe broker routing
- Thread-safe message queue with dispatcher thread
- `PING` / `PONG` command
- Idle client timeout
- Thread-safe logging
- Runtime metrics via `STATS`
- Graceful Ctrl+C shutdown
- Unit-style tests for parser, broker, and queue logic

## Architecture

```text
main.cpp
  Creates ServerConfig and starts TcpServer

TcpServer
  Listens for TCP clients
  Assigns client IDs
  Submits ClientSession work to the thread pool
  Starts the message dispatcher thread

ThreadPool
  Runs client sessions on a fixed number of worker threads

ClientSession
  Handles one connected client
  Parses commands
  Sends responses
  Pushes published messages into MessageQueue

MessageQueue
  Thread-safe producer-consumer queue
  Receives published messages from client sessions
  Feeds dispatcher thread

Broker
  Tracks topic subscriptions
  Routes messages to registered clients

MessageParser
  Converts raw text commands into structured commands

Metrics
  Tracks active clients, total clients, published messages, and delivered messages

Logger
  Serializes log output from multiple threads
```

## Protocol Commands

Connect using `nc` and type commands line by line.

```text
SUB <topic>
UNSUB <topic>
PUB <topic> <message>
PING
STATS
TOPICS
CLIENTS
HELP
quit
```

Examples:

```text
SUB market-data
PUB market-data AAPL=192.31
PING
STATS
TOPICS
CLIENTS
HELP
quit
```

## Build On Linux / WSL

Install dependencies:

```bash
sudo apt update
sudo apt install -y build-essential cmake libboost-all-dev
```

Build:

```bash
Clone the repository:
git clone https://github.com/Abhijeetgaurav/Multithreaded-TCP-Message-Broker.git
Create a build directory and compile:
mkdir -p build
cd build
cmake ..
cmake --build .
```

## Run The Server

```bash
cd build
./tcp_server
```

By default, the server listens on port `5555`. This is configured in:
The idle timeout and worker thread count are configured there too.

```text
include/ServerConfig.hpp
```

## Manual Demo

Open terminal 1:

```bash
./tcp_server
```

Open terminal 2:

```bash
nc localhost 5555
```

Subscribe to a topic:

```text
SUB market-data
```

Open terminal 3:

```bash
nc localhost 5555
```

Publish a message:

```text
PUB market-data AAPL=192.31
```

The subscribed client should receive:

```text
MSG market-data from client 2 AAPL=192.31
```

The publishing client should receive:

```text
OK queued market-data
```

## Response Format

The server uses a small line-based protocol:

```text
OK <details>      command succeeded
ERR <reason>      command failed
MSG <topic> ...   published message delivered to a subscriber
PONG              response to PING
```

## Metrics

From any connected client:

```text
STATS
```

Example response:

```text
active_clients=2
total_clients=2
messages_published=1
messages_delivered=1
```

## Tests

Run:

```bash
cd build
./broker_tests
```

Expected output:

```text
All broker/parser tests passed.
```

The tests cover:

- command parsing
- broker routing
- unsubscribe behavior
- message queue push/pop
- message queue shutdown

## Current Limitations

- Uses blocking client sessions, so each connected client occupies one worker thread while connected
- Uses a simple text protocol instead of JSON or length-prefixed frames
- Connected clients may remain active until they disconnect or hit the idle timeout during shutdown
- Message delivery is in-memory only
- No authentication or TLS

## Possible Next Improvements

- Replace blocking sessions with async Boost.Asio sessions for higher connection counts
- Add integration tests that start the server and connect test clients
- Add Dockerfile for easier Linux execution
