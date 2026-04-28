#include <cstring>
#include <stdexcept>
#include <iostream>

#include "socket/SocketServer.hpp"

SocketServer::SocketServer()
{
    std::memset(&_address, 0, sizeof(_address));
    _address.sin_family = AF_INET;
    _address.sin_port = htons(8080);
    _address.sin_addr.s_addr = INADDR_ANY;
}

SocketServer::~SocketServer() {}

struct sockaddr_in  SocketServer::getAddress()
{
    return (_address);
}

void SocketServer::bindSocket()
{
    if (bind(getFd(), (struct sockaddr*)&_address, sizeof(_address)) < 0)
        throw std::runtime_error("bind() failed");
}

void    SocketServer::listenSocket(int maxConnections)
{
    if (listen(getFd(), maxConnections) < 0)
        throw std::runtime_error("listen() failed");
}

int    SocketServer::acceptClient()
{
    int client_fd = accept(getFd(), NULL, NULL);
    if (client_fd < 0)
        throw std::runtime_error("accept() failed");
    std::cout << "client connected" << std::endl;

    return (client_fd);
}