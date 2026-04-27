#include "server/Client.hpp"

#include <fstream>
#include <sstream>

Client::Client(int fd) : _fd(fd) {}
Client::~Client(void) {}

int				Client::getFd(void) const { return _fd; }
std::string&	Client::getRecvBuf(void) { return _recv_buf; }
std::string&	Client::getSendBuf(void) { return _send_buf; };

std::string	Client::_parsePathFromRequest(void) const
{
	if (_recv_buf.find("GET / ") != std::string::npos)
		return "/";
	if (_recv_buf.find("GET /") != std::string::npos)
	{
		size_t start = _recv_buf.find("GET /") + 5;
		size_t end   = _recv_buf.find(" ", start);
		return _recv_buf.substr(start, end - start);
	}
	return "/";
}
 
std::string	Client::_readFile(const std::string& filepath) const
{
	std::ifstream		file(filepath.c_str());
	std::stringstream	ss;
	std::string			line;
 
	if (!file.is_open())
		return "";
	while (std::getline(file, line))
		ss << line << "\n";
	return ss.str();
}
 
void	Client::buildResponse(void)
{
	std::string path = _parsePathFromRequest();
 
	std::string filepath;
	if (path == "/")
		filepath = "www/index.html";
	else if (path == "test")
		filepath = "www/test";
 
	std::stringstream response;
 
	if (!filepath.empty())
	{
		std::string body = _readFile(filepath);
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
 
	_send_buf = response.str();
}

bool	Client::isRequestComplete(void) const
{
	return _recv_buf.find("\r\n\r\n") != std::string::npos;
}
