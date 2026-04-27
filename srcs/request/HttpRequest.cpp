#include "request/HttpRequest.hpp"

#include <sstream>
#include <cstdlib>

static std::string	toLower(const std::string& s)
{
	std::string	out = s;
	for (size_t	i = 0; i < out.size(); ++i)
		out[i] = std::tolower((unsigned char)out[i]);

	return out;
}

static std::string	trim(const std::string& s)
{
	size_t	start = s.find_first_not_of(" \t\r\n");
	if (start == std::string::npos) return "";
	size_t	end = s.find_last_not_of(" \t\r\n");

	return s.substr(start, end - start + 1);
}

bool	parseRequest(const std::string& raw, HttpRequest& req)
{
	// ── 1. Wait for end of headers ───────────────────────────────────────────
	size_t headerEnd = raw.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
		return false; // headers not fully received yet

	std::string headerSection = raw.substr(0, headerEnd);

	// ── 2. Parse request line (first line) ───────────────────────────────────
	size_t firstLine = headerSection.find("\r\n");
	std::string requestLine = headerSection.substr(0, firstLine);

	std::istringstream rl(requestLine);
	rl >> req.method >> req.path >> req.version;

	if (req.method.empty() || req.path.empty())
		return false;

	// ── 3. Parse headers ─────────────────────────────────────────────────────
	// Headers start after the request line
	size_t pos = firstLine + 2; // skip \r\n
	while (pos < headerSection.size())
	{
		size_t lineEnd = headerSection.find("\r\n", pos);
		if (lineEnd == std::string::npos) lineEnd = headerSection.size();

		std::string line = headerSection.substr(pos, lineEnd - pos);
		size_t colon = line.find(':');
		if (colon != std::string::npos)
		{
			std::string key = toLower(trim(line.substr(0, colon)));
			std::string val = trim(line.substr(colon + 1));
			req.headers[key] = val;
		}
		pos = lineEnd + 2;
	}

	// ── 4. Read body if present (POST) ───────────────────────────────────────
	size_t bodyStart = headerEnd + 4; // skip \r\n\r\n

	// Check Content-Length to know how many body bytes to expect
	std::map<std::string, std::string>::const_iterator it =
		req.headers.find("content-length");

	if (it != req.headers.end())
	{
		size_t contentLength = (size_t)std::atol(it->second.c_str());

		// Not all body bytes have arrived yet
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

	req.complete = true;
	return true;
}
