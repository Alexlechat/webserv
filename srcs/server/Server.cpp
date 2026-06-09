#include "server/Server.hpp"
#include "logger/FileLogger.hpp"

Server::Server(ServerConfig serverConfig)
	: SocketServer(serverConfig.port),
	  _serverConfig(serverConfig)
{

	_fileErrorLogger = new FileLogger(serverConfig.errorLogConfig);
	_fileAccessLogger = new FileLogger(serverConfig.accessLogConfig);

	bindSocket();
	listenSocket(LISTEN_MAX);
}

Server::~Server(void)
{
	delete _fileErrorLogger;
	delete _fileAccessLogger;
}
const FileLogger&	Server::getServerErrorLogger(void) const { return *_fileErrorLogger; }
const FileLogger&	Server::getServerAccessLogger(void) const { return *_fileAccessLogger; }
const ServerConfig&	Server::getServerConfig(void) const { return _serverConfig; }
