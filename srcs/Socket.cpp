#include <unistd.h> 
#include <stdexcept>
#include <sys/socket.h>

#include "Socket.hpp"

Socket::Socket()
{
    _fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_fd < 0)
        throw std::runtime_error("socket() failed");
}

Socket::~Socket()
{
    if (_fd >= 0)
        close(_fd);
}

int Socket::getFd()
{
    return (_fd);
}
