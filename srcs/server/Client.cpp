#include "server/Client.hpp"

#include <fstream>
#include <sstream>


Client::Client(int fd) : SocketClient(fd) {}
Client::~Client(void) {}

std::string& Client::getRecvBuf() { return _bufferRequest; }
std::string& Client::getSendBuf() { return _sendBuffer; }

bool	Client::isRequestComplete(void) const
{
	return _bufferRequest.find("\r\n\r\n") != std::string::npos;
}

std::string	Client::_parsePathFromRequest(void) const
{
	if (_bufferRequest.find("GET / ") != std::string::npos)
		return "/";
	if (_bufferRequest.find("GET /") != std::string::npos)
	{
		size_t start = _bufferRequest.find("GET /") + 5;
		size_t end   = _bufferRequest.find(" ", start);
		return _bufferRequest.substr(start, end - start);
	}
	return "/";
}

std::string	Client::_readFile(const std::string& filepath) const
{
	std::ifstream		file(filepath.c_str());
	std::stringstream	ss;
	std::string			line;

	if (!file.is_open())
		return ("");
	while (std::getline(file, line))
		ss << line << "\n";
	return ss.str();
}

void	Client::buildResponse(void)
{
	std::string path = _parsePathFromRequest();
	std::string filepath;

	if (path == "/")
		filepath = "www/html/index.html";
	else
		filepath = "www/html/" + path;

	std::stringstream 	response;
	std::string			body = _readFile(filepath);

	if (!body.empty())
	{
		response << "HTTP/1.1 200 OK\r\n"
				<< "Content-Type: text/html; charset=UTF-8\r\n"
				<< "Content-Length: " << body.size() << "\r\n"
				<< "\r\n"
				<< body;
	}
	else
	{
		std::string body404 = "<html><body><h1>404 Not Found</h1></body></html>";
		response << "HTTP/1.1 404 Not Found\r\n"
				<< "Content-Type: text/html; charset=UTF-8\r\n"
				<< "Content-Length: " << body404.size() << "\r\n"
				<< "\r\n"
				<< body404;
	}

	sendData(response.str());
}
