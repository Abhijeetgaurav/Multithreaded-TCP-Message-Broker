#include "Broker.hpp"

#include <algorithm>
#include <sstream>

void Broker::registerClient(int clientId, SendCallback callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    clients_[clientId] = std::move(callback);
}

void Broker::unregisterClient(int clientId)
{
    std::lock_guard<std::mutex> lock(mutex_);

    clients_.erase(clientId);

    for (auto it = topicSubscribers_.begin(); it != topicSubscribers_.end(); )
    {
        it->second.erase(clientId);

        if (it->second.empty())
        {
            it = topicSubscribers_.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void Broker::subscribe(int clientId, const std::string &topic)
{
    std::lock_guard<std::mutex>lock(mutex_);
    topicSubscribers_[topic].insert(clientId);
}

void Broker::unsubscribe(int clientId, const std::string &topic)
{
    std::lock_guard<std::mutex>lock(mutex_);

    auto it = topicSubscribers_.find(topic);
    if(it == topicSubscribers_.end())
    {
        return;
    }
    it->second.erase(clientId);

    if(it->second.empty())
    {
        topicSubscribers_.erase(it);
    }
}

std::size_t Broker::publish(int publisherId, const std::string& topic, const std::string& message)
{
    std::vector<SendCallback> callbacks;

    {
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = topicSubscribers_.find(topic);
        if (it == topicSubscribers_.end())
        {
            return 0;
        }

        for (int clientId : it->second)
        {
            auto clientIt = clients_.find(clientId);
            if (clientIt != clients_.end())
            {
                callbacks.push_back(clientIt->second);
            }
        }
    }

    std::string outgoing =
        "MSG " + topic + " from client " +
        std::to_string(publisherId) + " " +
        message + "\n";

    for (const auto& callback : callbacks)
    {
        callback(outgoing);
    }

    return callbacks.size();
}

std::string Broker::topicsSnapshot() const
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (topicSubscribers_.empty())
    {
        return "topics=0\n";
    }

    std::vector<std::string> topics;
    topics.reserve(topicSubscribers_.size());

    for (const auto& [topic, subscribers] : topicSubscribers_)
    {
        topics.push_back(topic + " subscribers=" + std::to_string(subscribers.size()));
    }

    std::sort(topics.begin(), topics.end());

    std::ostringstream output;
    output << "topics=" << topics.size() << "\n";

    for (const std::string& topicLine : topics)
    {
        output << topicLine << "\n";
    }

    return output.str();
}

std::string Broker::clientsSnapshot() const
{
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<int> clientIds;
    clientIds.reserve(clients_.size());

    for (const auto& [clientId, callback] : clients_)
    {
        (void)callback;
        clientIds.push_back(clientId);
    }

    std::sort(clientIds.begin(), clientIds.end());

    std::ostringstream output;
    output << "clients=" << clientIds.size() << "\n";

    for (int clientId : clientIds)
    {
        output << "client " << clientId << "\n";
    }

    return output.str();
}
