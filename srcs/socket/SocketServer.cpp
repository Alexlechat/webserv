#include <cstring>
#include <stdexcept>
#include <iostream>

#include "socket/SocketServer.hpp"

SocketServer::SocketServer(int port)
{
    int	opt = 1;
	setsockopt(getSocketFd(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    std::memset(&_address, 0, sizeof(_address));
    _address.sin_family = AF_INET;
    _address.sin_port = htons(port);
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

int    SocketServer::acceptClient()
{
    int client_fd = accept(getSocketFd(), NULL, NULL);
    if (client_fd < 0)
        throw std::runtime_error("accept() failed");
    std::cout << "client connected" << std::endl;

    return (client_fd);
}
