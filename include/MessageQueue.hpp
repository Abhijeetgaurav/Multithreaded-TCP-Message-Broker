#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>

struct QueuedMessage
{
    int publisherId;
    std::string topic;
    std::string payload;
};

class MessageQueue
{
public:
    void push(QueuedMessage message);
    bool waitAndPop(QueuedMessage& message);
    void shutdown();

private:
    std::mutex mutex_;
    std::condition_variable condition_;
    std::queue<QueuedMessage> queue_;
    bool stopped_{false};
};
