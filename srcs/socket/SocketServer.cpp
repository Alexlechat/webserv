#include <cstring>
#include <iostream>
#include <stdexcept>
#include <arpa/inet.h>

#include "socket/SocketServer.hpp"

SocketServer::SocketServer(uint16_t port, const std::string& host)
{

    int	opt = 1;
	setsockopt(getSocketFd(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    std::memset(&_address, 0, sizeof(_address));
    _address.sin_family = AF_INET;
    _address.sin_port = htons(port);

    if (!host.empty() && host != "0.0.0.0")
    {
        if (inet_pton(AF_INET, host.c_str(), &_address.sin_addr) != 1)
            throw std::runtime_error("Invalid listen address: " + host);
    }
    else
        _address.sin_addr.s_addr = INADDR_ANY;
}

SocketServer::~SocketServer() {}

struct sockaddr_in  SocketServer::getAddress()
{
    return (_address);
}

void SocketServer::bindSocket()
{
    if (bind(getSocketFd(), (struct sockaddr*)&_address, sizeof(_address)) < 0)
        throw std::runtime_error("bind() failed");
}

void    SocketServer::listenSocket(int maxConnections)
{
    if (listen(getSocketFd(), maxConnections) < 0)
        throw std::runtime_error("listen() failed");
}
