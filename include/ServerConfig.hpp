#pragma once

struct ServerConfig
{
    unsigned short port = 5555;
    int idleTimeoutSeconds = 60;
    int workerThreadCount = 4;
};
