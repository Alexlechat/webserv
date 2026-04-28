#include "server/Server.hpp"

#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>

Server::Server(ServerConfig serverConfig) : _fd(-1), _serverConfig(serverConfig)
{
	_initSocket(_serverConfig.port);
	_bindSocket();
	_listenSocket(LISTEN_MAX);
}

Server::~Server(void)
{
	if (_fd != -1)
		close(_fd);
}

void Server::_initSocket(int port)
{
	_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_fd == -1)
		throw ServerException("Socket creation failure");

	int	opt = 1;
	setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	_addr.sin_family = AF_INET;
	_addr.sin_addr.s_addr = INADDR_ANY;
	_addr.sin_port = htons(port);
}

void	Server::_bindSocket(void) const
{
	if (bind(_fd, (struct sockaddr *)&_addr, sizeof(_addr)) < 0)
		throw ServerException("Socket bind failure");
}

void	Server::_listenSocket(int maxConnections) const
{
	if (listen(_fd, maxConnections) < 0)
		throw ServerException("Socket listen failure");
}

int					Server::getServerFd(void) const { return _fd; }
const ServerConfig&	Server::getServerConfig(void) const { return _serverConfig; }
