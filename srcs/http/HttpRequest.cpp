#include "http/Http.hpp"
#include "http/HttpRequest.hpp"
#include "utils/Utils.hpp"
#include <cstdlib>
#include <sstream>

HttpRequest::HttpRequest(void)
	: methodEnum(Http::UNKNOWN_METHOD)
	, _state(PARSING_REQUEST_LINE)
	, _contentLenght(0)
	, _chunked(false)
	, _maxBodySize(0)
{}

void	HttpRequest::setMaxBodySize(size_t maxBodySize) { _maxBodySize = maxBodySize; }

HttpRequest::ParseState	HttpRequest::feed(const std::string& chunk)
{
	_buffer += chunk;

	while (_state != PARSING_COMPLETE && _state != PARSING_ERROR && _state != PARSING_TOO_LARGE)
	{
		ParseState	prev = _state;

		switch (_state)
		{
			case PARSING_REQUEST_LINE:	_state = _tryParseRequestLine(); break;
			case PARSING_HEADERS:		_state = _tryParseHeaders(); break;
			case PARSING_BODY:			_state = _chunked ? _tryParseChunkedBody() : _tryParseBody(); break;
			default: break;
		}

		if (_state == prev)
			break;
	}

	return _state;
}

HttpRequest::ParseState	HttpRequest::_tryParseRequestLine(void)
{
	size_t	lineEnd = _buffer.find(Http::CRLF);
	if (lineEnd == std::string::npos)
		return PARSING_REQUEST_LINE;

	std::istringstream	iss(_buffer.substr(0, lineEnd));
	iss >> method >> path >> version;

	if (method.empty() || path.empty() || version.empty())
		return PARSING_ERROR;

	size_t	q = path.find('?');
	if (q != std::string::npos)
	{
		query = path.substr(q + 1);
		path = path.substr(0, q);
	}

	methodEnum = Http::strToMethod(method);
	_buffer.erase(0, lineEnd + 2);
	return PARSING_HEADERS;
}

HttpRequest::ParseState	HttpRequest::_tryParseHeaders(void)
{
	size_t	headersEnd;
	size_t	blankLine = _buffer.find(Http::HEADER_END);

	if (blankLine != std::string::npos)
		headersEnd = blankLine;
	else if (_buffer.size() >= 2 && _buffer[0] == '\r' && _buffer[1] == '\n')
		headersEnd = 0;
	else
		return PARSING_HEADERS;

	size_t	pos = 0;
	while (pos < headersEnd)
	{
		size_t	lineEnd = _buffer.find(Http::CRLF, pos);
		if (lineEnd == std::string::npos || lineEnd > headersEnd)
			lineEnd = headersEnd; 

		size_t	colon = _buffer.find(':', pos);
		if (colon != std::string::npos && colon < lineEnd)
		{
			std::string	key = Utils::strToLower(Utils::trim(_buffer.substr(pos, colon - pos), Http::SPACE_CHARS));
			std::string	val = Utils::trim(_buffer.substr(colon + 1, lineEnd - colon - 1), Http::SPACE_CHARS);
			headers[key] = val;
		}
		pos = lineEnd + 2;
	} 

	_buffer.erase(0, headersEnd == 0 ? 2 : headersEnd + 4);

	std::map<std::string, std::string>::const_iterator	te = headers.find("transfer-encoding");
	if (te != headers.end() && te->second == "chunked")
	{
		_chunked = true;
		return PARSING_BODY;
	}

	std::map<std::string, std::string>::const_iterator	cl =  headers.find("content-length");
	if (cl != headers.end())
	{
		char*	end;
		long	val = std::strtol(cl->second.c_str(), &end, 10);
		if (*end != '\0' || val < 0)
			return PARSING_ERROR;

		_contentLenght = static_cast<size_t>(val);
		return (_contentLenght > 0) ? PARSING_BODY : PARSING_COMPLETE;
	}

	return PARSING_COMPLETE;
}

HttpRequest::ParseState	HttpRequest::_tryParseBody(void)
{
	if (_buffer.size() < _contentLenght)
		return PARSING_BODY;

	body = _buffer.substr(0, _contentLenght);
	_buffer.erase(0, _contentLenght);
	return PARSING_COMPLETE;
}

HttpRequest::ParseState	HttpRequest::_tryParseChunkedBody(void)
{
	while (true)
	{
		size_t	sizeEnd = _buffer.find(Http::CRLF);
		if (sizeEnd == std::string::npos)
			return PARSING_BODY;

		char*	end;
		long	chunkSizeVal = std::strtol(_buffer.substr(0, sizeEnd).c_str(), &end, 16);
		if (end == _buffer.c_str() || chunkSizeVal < 0)
			return PARSING_ERROR;

		size_t	chunkSize = static_cast<size_t>(chunkSizeVal);

		if (_buffer.size() < sizeEnd + 2 + chunkSize + 2)
			return PARSING_BODY;

		if (chunkSize == 0)
		{
			_buffer.erase(0, sizeEnd + 2 + 2);
			return PARSING_COMPLETE;
		}

		if (_maxBodySize > 0 && body.size() + chunkSize > _maxBodySize)
			return PARSING_TOO_LARGE;

		body += _buffer.substr(sizeEnd + 2, chunkSize);
		_buffer.erase(0, sizeEnd + 2 + chunkSize + 2);
	}
}

HttpRequest::ParseState	HttpRequest::state(void) const		{ return _state; }
bool					HttpRequest::isComplete(void) const	{ return _state == PARSING_COMPLETE; }
bool					HttpRequest::hasError(void) const	{ return _state == PARSING_ERROR || _state == PARSING_TOO_LARGE; }

void	HttpRequest::reset(void)
{
	method.clear();
	methodEnum = Http::UNKNOWN_METHOD;
	path.clear();
	query.clear();
	version.clear();
	headers.clear();
	body.clear();
	_buffer.clear();
	_state = PARSING_REQUEST_LINE;
	_contentLenght = 0;
	_chunked = false;
}
