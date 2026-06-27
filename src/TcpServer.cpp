#include "TcpServer.hpp"
#include "ClientSession.hpp"
#include <iostream>
#include <memory>
#include <thread>

using boost::asio::ip::tcp;

TcpServer::TcpServer(ServerConfig config)
    : io_(),
      acceptor_(io_, tcp::endpoint(tcp::v4(), config.port)),
      config_(config),
      nextClientId_(1),
      broker_(),
      metrics_(),
      logger_(),
      messageQueue_(),
      clientThreadPool_(config.workerThreadCount),
      running_(false)
{
}

void TcpServer::stop()
{
    bool wasRunning = running_.exchange(false);

    if (!wasRunning)
    {
        return;
    }

    logger_.info("Shutdown requested");

    boost::system::error_code error;
    acceptor_.close(error);

    if (error)
    {
        logger_.error("Failed to close acceptor: " + error.message());
    }

    messageQueue_.shutdown();
    io_.stop();
}

void TcpServer::dispatchMessages()
{
    logger_.info("Message dispatcher started");

    while (true)
    {
        QueuedMessage message;

        if (!messageQueue_.waitAndPop(message))
        {
            logger_.info("Message dispatcher stopped");
            break;
        }

        std::size_t deliveredCount = broker_.publish(
            message.publisherId,
            message.topic,
            message.payload
        );

        metrics_.messagesDelivered(deliveredCount);

        logger_.info(
            "Dispatched message from client " +
            std::to_string(message.publisherId) +
            " to topic " + message.topic +
            " for " + std::to_string(deliveredCount) +
            " subscriber(s)"
        );
    }
}
void TcpServer::start()
{
    logger_.info("Server listening on port " + std::to_string(config_.port));

    running_ = true;

    boost::asio::signal_set signals(io_, SIGINT, SIGTERM);
    signals.async_wait([this](const boost::system::error_code& error, int)
    {
        if (!error)
        {
            logger_.info("Shutdown signal received");
            stop();
        }
    });

    std::thread signalThread([this]()
    {
        io_.run();
    });

    dispatcherThread_ = std::thread([this]()
    {
        dispatchMessages();
    });

    while (running_)
    {
        tcp::socket socket(io_);

        boost::system::error_code error;
        acceptor_.accept(socket, error);

        if (error)
        {
            if (error == boost::asio::error::interrupted)
            {
                stop();
                break;
            }

            if (running_)
            {
                logger_.error("Accept error: " + error.message());
            }

            break;
        }

        int clientId = nextClientId_++;
        logger_.info("Client " + std::to_string(clientId) + " connected");
        auto clientSocket = std::make_shared<tcp::socket>(std::move(socket));

        clientThreadPool_.post(
            [this, clientId, clientSocket]()
            {
                ClientSession session(
                    clientId,
                    std::move(*clientSocket),
                    broker_,
                    metrics_,
                    logger_,
                    messageQueue_,
                    config_.idleTimeoutSeconds
                );
                session.start();
            }
        );

        logger_.info("Ready for next client");
    }

    stop();

    if (dispatcherThread_.joinable())
    {
        dispatcherThread_.join();
    }

    if (signalThread.joinable())
    {
        signalThread.join();
    }

    logger_.info("Server stopped");
}
