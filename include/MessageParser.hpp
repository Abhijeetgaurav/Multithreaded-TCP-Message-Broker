#pragma once

#include <string>

enum class CommandType
{
    Subscribe,
    Unsubscribe,
    Publish,
    Stats,
    Topics,
    Clients,
    Ping,
    Help,
    Quit,
    Invalid
};

struct Command
{
    CommandType type;
    std::string topic;
    std::string payload;
    std::string error;
};

class MessageParser
{
public:
    static Command parse(const std::string& input);

private:
    static std::string trimLineEndings(std::string value);
};
