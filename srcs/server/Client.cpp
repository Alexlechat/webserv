#include "server/Client.hpp"
#include "logger/Logger.hpp"
#include "request/Response.hpp"

#include <cstdlib>
#include <sstream>
#include <arpa/inet.h>

Client::Client(int fd, const ServerConfig& serverConfig, const FileLogger& accessLogger)
	: SocketClient(fd), _accessLogger(accessLogger), _serverConfig(serverConfig) {}

Client::~Client(void) {}

bool	Client::tryBuildResponse(void)
{
	if (!parseRequest(_recv_buf, _request))
		return false;

	_send_buf = buildResponse(_request, _serverConfig);

	_status = _send_buf.substr(9, 3);
	_ip = inet_ntoa(getSockAddr().sin_addr);
	size_t cl_pos = _send_buf.find("Content-Length: ");
	if (cl_pos != std::string::npos)
	{
		size_t start = cl_pos + 16;
		size_t end   = _send_buf.find("\r\n", start);
		_bytesBodySent = _send_buf.substr(start, end - start);
	}

	return true;
}

void	Client::logAccess(void) const
{
	std::ostringstream	oss;

	oss << _ip << " - - " << "\"" << _request.requestLineToStr()
		<< "\" " << _status  << " " << _bytesBodySent << " \"-\" " 
		<< _request.headers.find("user-agent")->second;
	LOG(_accessLogger, oss.str());
}
