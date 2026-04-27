#ifndef HttpRequest_HPP
# define HttpRequest_HPP

# include <string>
# include <map>

struct	HttpRequest
{
	std::string							method;
	std::string							path;
	std::string							version;
	std::map<std::string, std::string>	headers;
	std::string							body;
	bool								complete;

	HttpRequest(void) : complete(false) {};
};

bool	parseRequest(const std::string& raw, HttpRequest& req);

#endif  // HttpRequest_HPP
