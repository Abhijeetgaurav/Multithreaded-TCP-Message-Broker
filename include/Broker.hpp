#pragma once

#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <functional>

using SendCallback = std::function<void(const std::string &)>;
class Broker
{
public:
    using SendCallback = std::function<void(const std::string &)>;

    void registerClient(int clientId, SendCallback callback);
    void unregisterClient(int clientId);

    void subscribe(int clientId, const std::string &topic);
    void unsubscribe(int clientId, const std::string &topic);

    std::size_t publish(int publisherId, const std::string &topic, const std::string &message);
    std::string topicsSnapshot() const;
    std::string clientsSnapshot() const;

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::unordered_set<int>> topicSubscribers_;
    std::unordered_map<int, SendCallback> clients_;
};
