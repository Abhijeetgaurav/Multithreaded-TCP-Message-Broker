cmake_minimum_required(VERSION 3.16)

project(TcpMessageBroker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
find_package(GTest REQUIRED)

enable_testing()

add_executable(tcp_server 
    src/main.cpp
    src/TcpServer.cpp
    src/ClientSession.cpp
    src/Broker.cpp
    src/MessageParser.cpp
    src/Metrics.cpp
    src/Logger.cpp
    src/MessageQueue.cpp
    src/ThreadPool.cpp
)

add_executable(broker_tests
    tests/broker_tests.cpp
    src/Broker.cpp
    src/MessageParser.cpp
    src/MessageQueue.cpp
)

target_include_directories(tcp_server
    PRIVATE include
)

target_include_directories(broker_tests
    PRIVATE include
)
target_link_libraries(tcp_server
    PRIVATE 
    Boost::system
    Threads::Threads
)

target_link_libraries(broker_tests
    PRIVATE
    GTest::gtest_main
    Boost::system
    Threads::Threads
)

include(GoogleTest)
gtest_discover_tests(broker_tests)
