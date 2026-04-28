#include <stdexcept>

#include "SocketClient.hpp"

SocketClient::SocketClient(int fd) : Socket(fd) {}

SocketClient::~SocketClient() {}

std::string SocketClient::getBufferRequest()
{
    return (_bufferRequest);
}

int    SocketClient::receiveData()
{
    char    buffer[1024];
    int     bytes;

    bytes = recv(getFd(), buffer, sizeof(buffer), 0);
    if (bytes > 0)
        _bufferRequest.append(buffer, bytes);

    return (bytes);
}

int SocketClient::sendData(const std::string& response)
{
    return (send(getFd(), response.c_str(), response.size(), 0));
}