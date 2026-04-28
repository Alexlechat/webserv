#include "server/Client.hpp"
#include "request/Response.hpp"
#include "request/HttpRequest.hpp"

#include <iostream>

Client::Client(int fd, const ServerConfig& serverConfig) : SocketClient(fd), _serverConfig(serverConfig) {}
Client::~Client(void) {}

std::string& Client::getRecvBuf() { return _bufferRequest; }
std::string& Client::getSendBuf() { return _sendBuffer; }

void	Client::buildResponse(void)
{
	if (!parseRequest(_recv_buf, _request))
		return false;

	std::cout << "[" << _fd << "] " << _request.method << " " << _request.path << std::endl;

	_send_buf = buildResponse(_request, _serverConfig);
	return true;
}
