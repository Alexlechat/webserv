#include "server/Client.hpp"
#include "request/Response.hpp"
#include "request/HttpRequest.hpp"

#include <iostream>

Client::Client(int fd, const ServerConfig& serverConfig) : _fd(fd), _serverConfig(serverConfig) {}
Client::~Client(void) {}

int				Client::getFd(void) const { return _fd; }
std::string&	Client::getRecvBuf(void) { return _recv_buf; }
std::string&	Client::getSendBuf(void) { return _send_buf; };

bool			Client::tryBuildResponse(void)
{
	if (!parseRequest(_recv_buf, _request))
		return false;

	std::cout << "[" << _fd << "] " << _request.method << " " << _request.path << std::endl;

	_send_buf = buildResponse(_request, _serverConfig);
	return true;
}
