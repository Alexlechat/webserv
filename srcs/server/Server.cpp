#include "Server.hpp"

#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>

Server::Server(void)
{
	try
	{
		initSocket(DEFAULT_PORT);
		bindSocket();
		listenSocket(LISTEN_MAX);
	}
	catch (const std::exception& e)
	{
		std::cerr << "SERVER EXCEPTION: " << e.what() << std::endl;
	}
}

Server::~Server(void)
{
	close(_fd);
}

Server::Server(const Server& src) { *this = src; }
Server&	Server::operator=(const Server& rhs)
{
	if (this != &rhs)
	{
		_fd = rhs._fd;
		_addr = rhs._addr;
	}

	return *this;
}

int	Server::getServerFd(void) const
{
	return _fd;
}

void Server::initSocket(int port)
{
	_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_fd == -1)
		throw ServerException("Socket creation failure");

	_addr.sin_family = AF_INET;
	_addr.sin_addr.s_addr = INADDR_ANY;
	_addr.sin_port = htons(port);
}

void	Server::bindSocket(void) const
{
	if (bind(_fd, (struct sockaddr *)&_addr, sizeof(_addr)) < 0)
		throw ServerException("Socket bind failure");
}

void	Server::listenSocket(int maxConnections) const
{
	if (listen(_fd, maxConnections) < 0)
		throw ServerException("Socket listen failure");
}
