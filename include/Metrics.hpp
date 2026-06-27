#pragma once

#include <atomic>
#include <cstddef>
#include <string>

class Metrics
{
public:
    void clientConnected();
    void clientDisconnected();

    void messagePublished();
    void messagesDelivered(std::size_t count);

    std::string snapshot() const;

private:
    std::atomic<int> activeClients_{0};
    std::atomic<int> totalClients_{0};
    std::atomic<std::size_t> messagesPublished_{0};
    std::atomic<std::size_t> messagesDelivered_{0};
};