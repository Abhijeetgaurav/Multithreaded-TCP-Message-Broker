#pragma once

#include "Broker.hpp"
#include "Metrics.hpp"
#include "Logger.hpp"
#include "MessageQueue.hpp"
#include "ServerConfig.hpp"
#include "ThreadPool.hpp"

#include <boost/asio.hpp>
#include <atomic>
#include <thread>

class TcpServer
{
public:
    explicit TcpServer(ServerConfig config);

    void start();
    void stop();

private:
    boost::asio::io_context io_;
    boost::asio::ip::tcp::acceptor acceptor_;
    ServerConfig config_;
    std::atomic<int> nextClientId_;
    Broker broker_;
    Metrics metrics_;
    Logger logger_;
    MessageQueue messageQueue_;
    ThreadPool clientThreadPool_;
    std::atomic<bool> running_;
    std::thread dispatcherThread_;

    void dispatchMessages();
};
