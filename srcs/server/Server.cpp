#include "server/Server.hpp"
#include "logger/FileLogger.hpp"
#include <stdexcept>

Server::Server(const std::vector<ServerConfig>& serverConfigs)
	: SocketServer(serverConfigs.empty() ? 0 : serverConfigs[0].port,
				   serverConfigs.empty() || !serverConfigs[0].host.specified ? "" : serverConfigs[0].host.value),
	  _serverConfigs(serverConfigs)
{
	if (_serverConfigs.empty())
		throw std::runtime_error("Server: no ServerConfig provided");

	_fileErrorLogger = new FileLogger(_serverConfigs[0].error_log_config);
	_fileAccessLogger = new FileLogger(_serverConfigs[0].access_log_config);

	bindSocket();
	listenSocket(LISTEN_MAX);
}

Server::~Server(void)
{
	delete _fileErrorLogger;
	delete _fileAccessLogger;
}

const FileLogger&					Server::getServerErrorLogger(void) const { return *_fileErrorLogger; }
const FileLogger&					Server::getServerAccessLogger(void) const { return *_fileAccessLogger; }
const std::vector<ServerConfig>&	Server::getServerConfigs(void) const { return _serverConfigs; }
