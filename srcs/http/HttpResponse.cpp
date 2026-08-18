#include "http/HttpResponse.hpp"
#include "http/Http.hpp"
#include <sstream>
#include <string>

HttpResponse::HttpResponse(Http::StatusCode code) : _status(code), _omitBody(false) {}

void	HttpResponse::setStatus(Http::StatusCode code) { _status = code; }

void	HttpResponse::setHeader(const std::string& key, const std::string& value)
{
	_headers[key] = value;
}

void	HttpResponse::setBody(const std::string& body, const std::string& contentType)
{
	_body = body;
	_omitBody = false;

	if (!contentType.empty())
		_headers["content-type"] = contentType;

	std::ostringstream	oss;
	oss << body.size();
	_headers["content-length"] = oss.str();
}

void	HttpResponse::clearBody(void)
{
	_body.clear();
	_headers["content-length"] = "0";
}

void	HttpResponse::dropBodyForHead(void)
{
	_omitBody = true;
}

bool	HttpResponse::hasHeader(const std::string& key) const
{
	return _headers.find(key) != _headers.end();
}

std::string	HttpResponse::getHeader(const std::string& key) const
{
	std::map<std::string, std::string>::const_iterator	it = _headers.find(key);
	return (it == _headers.end()) ? "" : it->second;
}

std::string	HttpResponse::toString(void) const
{
	std::ostringstream	oss;

	oss << Http::VERSION << " " << _status << " "
		<< Http::statusToReason(_status) << Http::CRLF;

	for (std::map<std::string, std::string>::const_iterator	it = _headers.begin(); it != _headers.end(); ++it)
		oss << it->first << ": " << it->second << Http::CRLF;

	oss << Http::CRLF;
	if (!_omitBody)
		oss << _body;
	return oss.str();
}

Http::StatusCode	HttpResponse::status(void) const { return _status; }
