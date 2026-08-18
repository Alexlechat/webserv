#include "server/Client.hpp"
#include "logger/Logger.hpp"
#include "http/Http.hpp"
#include "http/HttpResponseBuilder.hpp"
#include "http/Cgi.hpp"
#include "utils/Utils.hpp"

#include <cstdlib>
#include <ctime>
#include <sstream>
#include <netinet/in.h>

Client::Client(int fd, const std::vector<ServerConfig>& serverConfigs, const FileLogger& accessLogger)
	: SocketClient(fd), _status(Http::OK), _accessLogger(accessLogger), _serverConfigs(serverConfigs),
	  _lastActivity(std::time(NULL)), _awaitingResponse(false), _keepAlive(false),
	  _forceClose(false), _cgi(NULL)
{
	size_t	coarseCap = 0; // 0 == unlimited
	for (size_t i = 0; i < _serverConfigs.size(); ++i)
	{
		size_t	limit = _serverConfigs[i].client_max_body_size;
		if (limit == 0)
		{
			coarseCap = 0;
			break;
		}
		if (limit > coarseCap)
			coarseCap = limit;
	}
	_request.setMaxBodySize(coarseCap);
}
Client::~Client(void) { delete _cgi; }

time_t	Client::getLastActivity(void) const { return _lastActivity; }
void	Client::touch(void) { _lastActivity = std::time(NULL); }
bool	Client::isAwaitingResponse(void) const { return _awaitingResponse; }
bool	Client::keepAlive(void) const { return _keepAlive; }
bool	Client::hasCgi(void) const { return _cgi != NULL; }
Cgi*	Client::getCgi(void) const { return _cgi; }

bool	Client::_wantsKeepAlive(void) const
{
	std::map<std::string, std::string>::const_iterator	it = _request.headers.find("connection");

	if (it != _request.headers.end())
		return Utils::strToLower(it->second) != "close";

	// HTTP/1.1 defaults to keep-alive, HTTP/1.0 defaults to close.
	return _request.version == "HTTP/1.1";
}

void	Client::resetForNextRequest(void)
{
	_request.reset();
	_response = HttpResponse();
	_send_buf.clear();
	_awaitingResponse = false;
	_keepAlive = false;
	_forceClose = false;
	_lastActivity = std::time(NULL);
}

void	Client::_captureIp(void)
{
	_ip = Utils::ipv4ToString(ntohl(getSockAddr().sin_addr.s_addr));
}

Client::FeedResult	Client::feed(const char *buf, size_t n)
{
	_lastActivity = std::time(NULL);

	HttpRequest::ParseState	state = _request.feed(std::string(buf, n));

	if (state == HttpRequest::PARSING_TOO_LARGE)
	{
		_onParseError(Http::CONTENT_TOO_LARGE);
		return FEED_RESPONSE_READY;
	}
	if (state == HttpRequest::PARSING_URI_TOO_LONG)
	{
		_onParseError(Http::URI_TOO_LONG);
		return FEED_RESPONSE_READY;
	}
	if (state == HttpRequest::PARSING_HEADERS_TOO_LARGE)
	{
		_onParseError(Http::HEADERS_TOO_LARGE);
		return FEED_RESPONSE_READY;
	}
	if (state == HttpRequest::PARSING_ERROR)
	{
		_onParseError(Http::BAD_REQUEST);
		return FEED_RESPONSE_READY;
	}
	if (state == HttpRequest::PARSING_COMPLETE)
		return _onRequestComplete();

	return FEED_CONTINUE;
}

const ServerConfig&	Client::_selectConfig(void) const
{
	std::map<std::string, std::string>::const_iterator	it = _request.headers.find("host");

	if (it != _request.headers.end())
	{
		std::string	host = it->second;
		size_t		colon = host.find(':');
		if (colon != std::string::npos)
			host = host.substr(0, colon);
		host = Utils::strToLower(Utils::trim(host, Http::SPACE_CHARS));

		for (size_t i = 0; i < _serverConfigs.size(); ++i)
		{
			if (!_serverConfigs[i].server_name->empty() &&
				Utils::strToLower(_serverConfigs[i].server_name) == host)
				return _serverConfigs[i];
		}
	}

	return _serverConfigs[0];
}

Client::FeedResult	Client::_onRequestComplete(void)
{
	// HTTP/1.1 requires a Host header (RFC 7230 §5.4).
	if (_request.version == "HTTP/1.1"
		&& _request.headers.find("host") == _request.headers.end())
	{
		_onParseError(Http::BAD_REQUEST);
		return FEED_RESPONSE_READY;
	}

	const ServerConfig&					cfg = _selectConfig();
	HttpResponseBuilder::BuildResult	result = buildResponse(_request, cfg);

	_captureIp();

	if (result.isCgi)
	{
		_cgi = result.cgi;
		return FEED_CGI_STARTED;
	}

	_response = result.response;
	_finalizeResponse();
	return FEED_RESPONSE_READY;
}

void	Client::_onParseError(Http::StatusCode code)
{
	_captureIp();
	// RFC 7230 6.3.1: once the parser lost track of the message
	// boundaries, whatever follows on this connection cannot be
	// trusted to be a new request. Answer, then close.
	_forceClose = true;
	_response = HttpResponse(code);
	HttpResponseBuilder::buildDefaultErrorPageBody(_response, code);
	_finalizeResponse();
}

pid_t	Client::finishCgi(void)
{
	if (!_cgi)
		return -1;

	// RFC 3875: the script answers on stdout. Nothing at all means the
	// interpreter was missing or the script died before writing a byte;
	// a non-zero exit means it gave up. Either way this is a gateway
	// failure, not an empty 200.
	if (_cgi->failed())
	{
		_response = HttpResponse(Http::BAD_GATEWAY);
		HttpResponseBuilder::buildDefaultErrorPageBody(_response, Http::BAD_GATEWAY);
	}
	else
		_response = _cgi->buildResponse();

	pid_t	pending = _cgi->pendingPid();
	delete _cgi;
	_cgi = NULL;

	if (_request.methodEnum == Http::HEAD)
		_response.dropBodyForHead();

	_finalizeResponse();
	return pending;
}

void	Client::abortCgi(Http::StatusCode code)
{
	delete _cgi;
	_cgi = NULL;

	_response = HttpResponse(code);
	HttpResponseBuilder::buildDefaultErrorPageBody(_response, code);
	_finalizeResponse();
}

void	Client::logAccess(void) const
{
	std::map<std::string, std::string>::const_iterator	ua =
        _request.headers.find("user-agent");

    std::ostringstream oss;
    oss << _ip << " - - "
        << "\"" << _request.method << " " << _request.path
        << " " << _request.version << "\" "
        << (int)_status << " "
        << (_bytesBodySent.empty() ? "0" : _bytesBodySent) << " \"-\" "
        << (ua != _request.headers.end() ? ua->second : "-");;
	LOG(_accessLogger, oss.str());
}

void	Client::_finalizeResponse(void)
{
	_keepAlive = !_forceClose && _wantsKeepAlive();
	_response.setHeader("connection", _keepAlive ? "keep-alive" : "close");

	// RFC 7230 3.3.2: a response that may carry a body must say how
	// long it is, otherwise the client keeps waiting for bytes that
	// never come. Responses built without a body (redirects, CGI
	// failures) would otherwise ship no framing at all.
	// 1xx/204/304 must not carry the header.
	int	code = (int)_response.status();
	if (code >= 200 && code != Http::NO_CONTENT && code != Http::NOT_MODIFIED
		&& !_response.hasHeader("content-length"))
		_response.setHeader("content-length", "0");

	// RFC 7231 §7.1.1.2: servers with a clock MUST send a Date header.
	char		dateBuf[64];
	std::time_t	now = std::time(NULL);
	struct tm*	gmt = gmtime(&now);
	std::strftime(dateBuf, sizeof(dateBuf), "%a, %d %b %Y %H:%M:%S GMT", gmt);
	_response.setHeader("date", dateBuf);

	_send_buf = _response.toString();
	_awaitingResponse = true;
	_status = _response.status();
	_bytesBodySent = _response.hasHeader("content-length") ? _response.getHeader("content-length") : "0";
	_lastActivity = std::time(NULL);
}
