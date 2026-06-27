#include "TcpServer.hpp"
#include "ServerConfig.hpp"

#include <iostream>

int main()
{
    try
    {
        ServerConfig config;
        TcpServer server(config);
        server.start();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}
