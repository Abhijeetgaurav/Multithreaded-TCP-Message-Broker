#include "Metrics.hpp"

#include <sstream>

void Metrics::clientConnected()
{
    ++activeClients_;
    ++totalClients_;
}

void Metrics::clientDisconnected()
{
    --activeClients_;
}

void Metrics::messagePublished()
{
    ++messagesPublished_;
}

void Metrics::messagesDelivered(std::size_t count)
{
    messagesDelivered_ += count;
}

std::string Metrics::snapshot() const
{
    std::ostringstream output;

    output << "active_clients=" << activeClients_.load() << "\n"
           << "total_clients=" << totalClients_.load() << "\n"
           << "messages_published=" << messagesPublished_.load() << "\n"
           << "messages_delivered=" << messagesDelivered_.load() << "\n";

    return output.str();
}