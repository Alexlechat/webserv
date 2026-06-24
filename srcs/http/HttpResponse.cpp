#include "http/HttpResponse.hpp"
#include "http/Http.hpp"
#include <sstream>
#include <string>

HttpResponse::HttpResponse(Http::StatusCode code) : _status(code) {}

void	HttpResponse::setStatus(Http::StatusCode code) { _status = code; }

void	HttpResponse::setHeader(const std::string& key, const std::string& value)
{
	_headers[key] = value;
}

void	HttpResponse::setBody(const std::string& body, const std::string& contentType)
{
	_body = body;

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

std::string	HttpResponse::toString(void) const
{
	std::ostringstream	oss;

	oss << Http::VERSION << " " << _status << " "
		<< Http::statusToReason(_status) << Http::CRLF;

	for (std::map<std::string, std::string>::const_iterator	it = _headers.begin(); it != _headers.end(); ++it)
		oss << it->first << ": " << it->second << Http::CRLF; 

	oss << Http::CRLF << _body;
	return oss.str();
}

Http::StatusCode	HttpResponse::status(void) const { return _status; }
