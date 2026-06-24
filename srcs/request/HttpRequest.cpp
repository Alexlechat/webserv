#include "utils/Utils.hpp"
#include "request/HttpRequest.hpp"

#include <sstream>
#include <cstdlib>

static bool	parseHeader(const std::string& raw, HttpRequest& req, size_t headerEnd);
static bool	parseBody(const std::string& raw, HttpRequest& req, size_t headerEnd);

bool	parseRequest(const std::string& raw, HttpRequest& req)
{
	size_t headerEnd = raw.find(END_OF_HEADER);
	if (headerEnd == std::string::npos)
		return false;

	if (!parseHeader(raw, req, headerEnd) || !parseBody(raw, req, headerEnd))
		return false;

	req.complete = true;
	return true;
}

static bool	parseHeader(const std::string& raw, HttpRequest& req, size_t headerEnd)
{
	std::string	headerSection = raw.substr(0, headerEnd);
	size_t		firstLine = headerSection.find(END_OF_LINE);
	std::string	requestLine = headerSection.substr(0, firstLine);

	std::istringstream	rl(requestLine);
	rl >> req.method >> req.path >> req.version;

	if (req.method.empty() || req.path.empty())
		return false;

	size_t q = req.path.find('?');
    if (q != std::string::npos)
    {
        req.query_string = req.path.substr(q + 1);
        req.path         = req.path.substr(0, q);
    }

	size_t	pos = firstLine + 2;
	while (pos < headerSection.size())
	{
		size_t	lineEnd = headerSection.find(END_OF_LINE, pos);
		if (lineEnd == std::string::npos) lineEnd = headerSection.size();

		std::string	line = headerSection.substr(pos, lineEnd - pos);
		size_t		colon = line.find(':');
		if (colon != std::string::npos)
		{
			std::string	key = Utils::strToLower(Utils::trim(line.substr(0, colon), SPACE_CHARS));
			std::string	val = Utils::trim(line.substr(colon + 1), SPACE_CHARS);
			req.headers[key] = val;
		}
		pos = lineEnd + 2;
	}

	return true;
}

static bool	parseBody(const std::string& raw, HttpRequest& req, size_t headerEnd)
{
	size_t	bodyStart = headerEnd + 4;

	std::map<std::string, std::string>::const_iterator	it = req.headers.find("content-length");
	if (it != req.headers.end())
	{
		size_t	contentLength = (size_t)std::atol(it->second.c_str());

		if (raw.size() - bodyStart < contentLength)
			return false;

		req.body = raw.substr(bodyStart, contentLength);
	}
	else if (req.method == "POST")
	{
		// No Content-Length — chunked or connection close.
		// For now, take whatever body is there.
		req.body = raw.substr(bodyStart);
	}

	return true;
}

std::string	HttpRequest::requestLineToStr(void) const
{
	std::string	out = method + " " + path + " " + version;

	return out;
}
