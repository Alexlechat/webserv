#include <fcntl.h>
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

Socket::Socket(int fd) : _fd(fd) {}

Socket::~Socket(void)
{
    if (_fd >= 0)
        close(_fd);
}

int Socket::getSocketFd(void) const { return (_fd); }

void	Socket::setNonBlocking(void) const
{
	if (fcntl(_fd, F_SETFL, O_NONBLOCK) == -1)
		throw std::runtime_error("fcntl() failed");
}
