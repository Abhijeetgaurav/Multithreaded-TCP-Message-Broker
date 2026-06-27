#include "Logger.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

void Logger::info(const std::string& message)
{
    log("INFO", message);
}

void Logger::error(const std::string& message)
{
    log("ERROR", message);
}

void Logger::log(const std::string& level, const std::string& message)
{
    auto now = std::chrono::system_clock::now();
    std::time_t nowTime = std::chrono::system_clock::to_time_t(now);

    std::lock_guard<std::mutex> lock(mutex_);

    std::cout
        << std::put_time(std::localtime(&nowTime), "%Y-%m-%d %H:%M:%S")
        << " [" << level << "] "
        << message
        << std::endl;
}