#include "MessageQueue.hpp"

void MessageQueue::push(QueuedMessage message)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (stopped_)
        {
            return;
        }

        queue_.push(std::move(message));
    }

    condition_.notify_one();
}

bool MessageQueue::waitAndPop(QueuedMessage& message)
{
    std::unique_lock<std::mutex> lock(mutex_);

    condition_.wait(lock, [this]
    {
        return stopped_ || !queue_.empty();
    });

    if (stopped_ && queue_.empty())
    {
        return false;
    }

    message = std::move(queue_.front());
    queue_.pop();

    return true;
}

void MessageQueue::shutdown()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stopped_ = true;
    }

    condition_.notify_all();
}
