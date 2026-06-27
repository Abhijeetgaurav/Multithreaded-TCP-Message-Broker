#pragma once

#include <mutex>
#include <string>

class Logger
{
public:
    void info(const std::string& message);
    void error(const std::string& message);

private:
    void log(const std::string& level, const std::string& message);

private:
    std::mutex mutex_;
};