#include "server/Server.hpp"

#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>

Server::Server(int port) : SocketServer(port)
{
	bindSocket();
	listenSocket(LISTEN_MAX);
}

Server::~Server(void) {}
