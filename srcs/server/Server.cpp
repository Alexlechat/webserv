#include "server/Server.hpp"

#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>

Server::Server(ServerConfig serverConfig) : SocketServer(serverConfig.port)
{
	bindSocket();
	listenSocket(LISTEN_MAX);
}

Server::~Server(void) {}

const ServerConfig&	Server::getServerConfig(void) const { return _serverConfig; }
