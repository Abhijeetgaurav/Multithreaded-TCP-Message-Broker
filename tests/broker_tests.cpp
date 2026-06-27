#include <gtest/gtest.h>

#include "Broker.hpp"
#include "MessageParser.hpp"
#include "MessageQueue.hpp"

TEST(MessageParserTest, SubscribeCommand)
{
    Command command = MessageParser::parse("SUB market-data\n");

    EXPECT_EQ(command.type, CommandType::Subscribe);
    EXPECT_EQ(command.topic, "market-data");
    EXPECT_TRUE(command.payload.empty());
}

TEST(MessageParserTest, PublishCommand)
{
    Command command = MessageParser::parse(
        "PUB market-data AAPL=192.31\n");

    EXPECT_EQ(command.type, CommandType::Publish);
    EXPECT_EQ(command.topic, "market-data");
    EXPECT_EQ(command.payload, "AAPL=192.31");
}

TEST(MessageParserTest, InvalidPublishCommand)
{
    Command command = MessageParser::parse("PUB market-data\n");

    EXPECT_EQ(command.type, CommandType::Invalid);
}

TEST(BrokerTest, RoutesToSubscribers)
{
    Broker broker;

    int deliveryCount = 0;

    broker.registerClient(1,
        [&](const std::string& message)
        {
            ++deliveryCount;
            EXPECT_EQ(
                message,
                "MSG market-data from client 2 AAPL=192.31\n");
        });

    broker.subscribe(1, "market-data");

    auto delivered =
        broker.publish(2, "market-data", "AAPL=192.31");

    EXPECT_EQ(delivered, 1u);
    EXPECT_EQ(deliveryCount, 1);
}

TEST(BrokerTest, DoesNotRouteToUnsubscribedClients)
{
    Broker broker;

    int deliveryCount = 0;

    broker.registerClient(1,
        [&](const std::string&)
        {
            ++deliveryCount;
        });

    broker.subscribe(1, "market-data");
    broker.unsubscribe(1, "market-data");

    auto delivered =
        broker.publish(2, "market-data", "AAPL=192.31");

    EXPECT_EQ(delivered, 0u);
    EXPECT_EQ(deliveryCount, 0);
}
TEST(BrokerTest, Snapshots)
{
    Broker broker;

    broker.registerClient(1, [](const std::string&) {});
    broker.registerClient(2, [](const std::string&) {});

    broker.subscribe(1, "market-data");
    broker.subscribe(2, "market-data");

    EXPECT_EQ(
        broker.clientsSnapshot(),
        "clients=2\nclient 1\nclient 2\n");

    EXPECT_EQ(
        broker.topicsSnapshot(),
        "topics=1\nmarket-data subscribers=2\n");
}

TEST(MessageQueueTest, PushPop)
{
    MessageQueue queue;

    queue.push({42, "market-data", "AAPL=192.31"});

    QueuedMessage message;

    bool result = queue.waitAndPop(message);

    EXPECT_TRUE(result);
    EXPECT_EQ(message.publisherId, 42);
    EXPECT_EQ(message.topic, "market-data");
    EXPECT_EQ(message.payload, "AAPL=192.31");
}

TEST(MessageQueueTest, Shutdown)
{
    MessageQueue queue;

    queue.shutdown();

    QueuedMessage message;

    EXPECT_FALSE(queue.waitAndPop(message));
}

TEST(MessageParserTest, PingCommand)
{
    auto cmd = MessageParser::parse("PING\n");

    EXPECT_EQ(cmd.type, CommandType::Ping);
}

TEST(MessageParserTest, HelpCommand)
{
    auto cmd = MessageParser::parse("HELP\n");

    EXPECT_EQ(cmd.type, CommandType::Help);
}

TEST(MessageParserTest, TopicsCommand)
{
    auto cmd = MessageParser::parse("TOPICS\n");

    EXPECT_EQ(cmd.type, CommandType::Topics);
}

TEST(MessageParserTest, ClientsCommand)
{
    auto cmd = MessageParser::parse("CLIENTS\n");

    EXPECT_EQ(cmd.type, CommandType::Clients);
}