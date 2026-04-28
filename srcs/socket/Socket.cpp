#include <unistd.h> 
#include <stdexcept>
#include <sys/socket.h>

#include "socket/Socket.hpp"

Socket::Socket(void)
{
    _fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_fd < 0)
        throw std::runtime_error("socket() failed");
}

Socket::~Socket(void)
{
    if (_fd >= 0)
        close(_fd);
}

Socket::Socket(int fd) : _fd(fd) {}

int Socket::getSocketFD() const { return (_fd); }
