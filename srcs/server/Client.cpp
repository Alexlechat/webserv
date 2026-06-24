#include "server/Client.hpp"
#include "logger/Logger.hpp"
#include "http/Http.hpp"
#include "http/HttpResponse.hpp"
#include "utils/Utils.hpp"

#include <cstdlib>
#include <sstream>
#include <arpa/inet.h>

Client::Client(int fd, const ServerConfig& serverConfig, const FileLogger& accessLogger)
	: SocketClient(fd), _accessLogger(accessLogger), _serverConfig(serverConfig) {}

Client::~Client(void) {}

bool	Client::feed(const char *buf, size_t n)
{
	HttpRequest::ParseState	state = _request.feed(std::string(buf, n));

	if (state == HttpRequest::PARSING_ERROR)
	{
		_onParseError(Http::BAD_REQUEST);
		return true;
	}
	if (state == HttpRequest::PARSING_COMPLETE)
	{
		_onRequestComplete();
		return true;
	}

	return false;
}

void	Client::_onRequestComplete(void)
{
	_ip = inet_ntoa(getSockAddr().sin_addr);
	_send_buf = buildResponse(_request, _serverConfig);

	if (_send_buf.size() > 12)
		_status = _send_buf.substr(9, 3);

	size_t	cl_pos = _send_buf.find("Content-length:");

	LOG_DEBUG(ConsoleLogger::instance(), _send_buf);
	LOG_DEBUG(ConsoleLogger::instance(), Utils::toString(cl_pos));

	if (cl_pos != std::string::npos)
	{
		size_t	start = cl_pos + 16;
		size_t	end = _send_buf.find("\r\n", start);
		_bytesBodySent = _send_buf.substr(start, end - start);
	}
}

void	Client::_onParseError(Http::StatusCode code)
{
	(void)code;
	_ip = inet_ntoa(getSockAddr().sin_addr);
	// HttpResponse	resp(code);
	// _send_buf = resp.toString();
	_status = "400";
}

void	Client::logAccess(void) const
{
	std::map<std::string, std::string>::const_iterator	ua =
        _request.headers.find("user-agent");

    std::ostringstream oss;
    oss << _ip << " - - "
        << "\"" << _request.method << " " << _request.path
        << " " << _request.version << "\" "
        << _status << " "
        << (_bytesBodySent.empty() ? "0" : _bytesBodySent) << " \"-\" "
        << (ua != _request.headers.end() ? ua->second : "-");;
	LOG(_accessLogger, oss.str());
}
