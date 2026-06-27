#include "MessageParser.hpp"

std::string MessageParser::trimLineEndings(std::string value)
{
    while (!value.empty() && (value.back() == '\n' || value.back() == '\r'))
    {
        value.pop_back();
    }

    return value;
}

Command MessageParser::parse(const std::string& input)
{
    std::string line = trimLineEndings(input);

    if (line == "quit" || line == "QUIT")
    {
        return {CommandType::Quit, "", "", ""};
    }

    if (line.rfind("SUB ", 0) == 0)
    {
        std::string topic = line.substr(4);

        if (topic.empty())
        {
            return {CommandType::Invalid, "", "", "Missing topic for SUB"};
        }

        return {CommandType::Subscribe, topic, "", ""};
    }

    if (line.rfind("UNSUB ", 0) == 0)
    {
        std::string topic = line.substr(6);

        if (topic.empty())
        {
            return {CommandType::Invalid, "", "", "Missing topic for UNSUB"};
        }

        return {CommandType::Unsubscribe, topic, "", ""};
    }

    if (line.rfind("PUB ", 0) == 0)
    {
        std::string rest = line.substr(4);
        std::size_t spacePos = rest.find(' ');

        if (spacePos == std::string::npos)
        {
            return {CommandType::Invalid, "", "", "Use: PUB <topic> <message>"};
        }

        std::string topic = rest.substr(0, spacePos);
        std::string payload = rest.substr(spacePos + 1);

        if (topic.empty() || payload.empty())
        {
            return {CommandType::Invalid, "", "", "Use: PUB <topic> <message>"};
        }

        return {CommandType::Publish, topic, payload, ""};
    }
    if (line == "STATS" || line == "stats")
    {
        return {CommandType::Stats, "", "", ""};
    }
    if (line == "TOPICS" || line == "topics")
    {
        return {CommandType::Topics, "", "", ""};
    }
    if (line == "CLIENTS" || line == "clients")
    {
        return {CommandType::Clients, "", "", ""};
    }
    if(line == "PING" || line == "ping")
    {
        return {CommandType::Ping, "","",""};
    }
    if (line == "HELP" || line == "help")
    {
        return {CommandType::Help, "", "", ""};
    }
    return {CommandType::Invalid, "", "", "Unknown command"};
}
