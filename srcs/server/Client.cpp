#include "server/Client.hpp"
#include "logger/Logger.hpp"
#include "request/Response.hpp"

Client::Client(int fd, const ServerConfig& serverConfig, const FileLogger& accessLogger)
	: SocketClient(fd), _accessLogger(accessLogger), _serverConfig(serverConfig) {}

Client::~Client(void) {}

bool	Client::tryBuildResponse(void)
{
	if (!parseRequest(_recv_buf, _request))
		return false;

	_send_buf = buildResponse(_request, _serverConfig);
	return true;
}

void	Client::logAccess(void) const { LOG(_accessLogger, _request.requestLineToStr()); }
