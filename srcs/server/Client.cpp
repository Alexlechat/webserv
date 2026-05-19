#include "server/Client.hpp"
#include "request/Response.hpp"
#include "request/HttpRequest.hpp"

Client::Client(int fd, const ServerConfig& serverConfig)
	: SocketClient(fd), _serverConfig(serverConfig) {}
Client::~Client(void) {}

bool	Client::tryBuildResponse(void)
{
	if (!parseRequest(_recv_buf, _request))
		return false;

	_send_buf = buildResponse(_request, _serverConfig);
	return true;
}
