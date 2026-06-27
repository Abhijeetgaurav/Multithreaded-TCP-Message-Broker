#include "ClientSession.hpp"
#include "MessageParser.hpp"
#include <array>
#include <iostream>
#include <string>
#include <cerrno>
#include <cstring>
#include <sys/select.h>
// remember a socket can't be copied,so we accept it by value and move it in to the class
ClientSession::ClientSession(
    int clientId,
    tcp::socket socket,
    Broker& broker,
    Metrics& metrics,
    Logger& logger,
    MessageQueue& messageQueue,
    int idleTimeoutSeconds
)
    : clientId_(clientId),
      socket_(std::move(socket)),
      broker_(broker),
      metrics_(metrics),
      logger_(logger),
      messageQueue_(messageQueue),
      idleTimeoutSeconds_(idleTimeoutSeconds)

{
}

void ClientSession::start()
{
    metrics_.clientConnected();
    broker_.registerClient(clientId_, [this](const std::string& message)
    {
        send(message);
    });

    bool running = true;

    while (running)
    {
        std::array<char, 1024> buffer;

        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(socket_.native_handle(), &readSet);

        timeval timeout;
        timeout.tv_sec = idleTimeoutSeconds_;
        timeout.tv_usec = 0;

        int ready = select(
            socket_.native_handle() + 1,
            &readSet,
            nullptr,
            nullptr,
            &timeout
        );

        if (ready == 0)
        {
            logger_.info("Client " + std::to_string(clientId_) + " idle timeout");
            break;
        }

        if (ready < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }

            logger_.error(
                "Client " + std::to_string(clientId_) +
                " select error: " + std::strerror(errno)
            );
            break;
        }

        boost::system::error_code error;
        std::size_t length = socket_.read_some(boost::asio::buffer(buffer), error);

        if (error == boost::asio::error::eof)
        {
            logger_.info("Client " + std::to_string(clientId_) + " disconnected");
            break;
        }
        else if (error)
        {
            logger_.error("Client " + std::to_string(clientId_) + " read error: " + error.message());
            break;
        }

        std::string message(buffer.data(), length);
        Command command = MessageParser::parse(message);

        switch (command.type)
        {
        case CommandType::Subscribe:
            broker_.subscribe(clientId_, command.topic);
            send("OK subscribed " + command.topic + "\n");
            break;

        case CommandType::Unsubscribe:
            broker_.unsubscribe(clientId_, command.topic);
            send("OK unsubscribed " + command.topic + "\n");
            break;

        case CommandType::Publish:
        {
            metrics_.messagePublished();

            messageQueue_.push({
                clientId_,
                command.topic,
                command.payload
            });

            send("OK queued " + command.topic + "\n");

            logger_.info(
                "Client " + std::to_string(clientId_) +
                " queued message for topic " + command.topic
            );
            break;
        }
        case CommandType::Stats:
            send(metrics_.snapshot());
            break;
        case CommandType::Topics:
            send(broker_.topicsSnapshot());
            break;
        case CommandType::Clients:
            send(broker_.clientsSnapshot());
            break;
        case CommandType::Ping:
            send("PONG\n");
            break;
        case CommandType::Help:
            send(
                "Commands:\n"
                "SUB <topic>\n"
                "UNSUB <topic>\n"
                "PUB <topic> <message>\n"
                "PING\n"
                "STATS\n"
                "TOPICS\n"
                "CLIENTS\n"
                "HELP\n"
                "quit\n"
            );
            break;
        case CommandType::Quit:
            logger_.info("Client " + std::to_string(clientId_) + " requested disconnect");
            send("OK goodbye\n");
            running = false;
            break;
        
        case CommandType::Invalid:
            send("ERR " + command.error + "\n");
            break;
        }
    }

    broker_.unregisterClient(clientId_);
    metrics_.clientDisconnected();
}

void ClientSession::send(const std::string& message)
{
    std::lock_guard<std::mutex> lock(writeMutex_);
    boost::asio::write(socket_, boost::asio::buffer(message));
}
