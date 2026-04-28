#ifndef Client_HPP
# define Client_HPP

# include "config/ServerConfig.hpp"
# include "request/HttpRequest.hpp"

# include <string>

class	Client
{
	public:
		Client(int fd, const ServerConfig& serverConfig);
		~Client(void);

		int				getFd(void) const;
		std::string&	getRecvBuf(void);
		std::string&	getSendBuf(void);

		bool			tryBuildResponse(void);

	private:
		int					_fd;
		std::string			_recv_buf;
		std::string			_send_buf;
		HttpRequest			_request;
		const ServerConfig&	_serverConfig;

		std::string		_parsePathFromRequest(void) const;
		std::string		_readFile(const std::string& filepath) const;
};

#endif  // Client_HPP
