#pragma once

#include "Broker.hpp"
#include <boost/asio.hpp>
#include "Metrics.hpp"
#include "Logger.hpp"
#include "MessageQueue.hpp"

#include <mutex>
#include <string>
using boost::asio::ip::tcp;
class ClientSession
{
public:
    explicit ClientSession(
        int clientId,
        tcp::socket socket,
        Broker& broker,
        Metrics& metrics,
        Logger& logger,
        MessageQueue& messageQueue,
        int idleTimeoutSeconds
    );

    void start();

private:
    void send(const std::string& message);
private:
    int clientId_;
    tcp::socket socket_;
    Broker& broker_;
    std::mutex writeMutex_;
    Metrics& metrics_;
    Logger& logger_;
    MessageQueue& messageQueue_;
    int idleTimeoutSeconds_;
};
