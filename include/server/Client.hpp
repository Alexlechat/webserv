#ifndef Client_HPP
# define Client_HPP

# include "config/ServerConfig.hpp"
# include "request/HttpRequest.hpp"

# include <string>

# include "socket/SocketClient.hpp"

class	Client : public SocketClient
{
	public:
		Client(int fd, const ServerConfig& serverConfig);
		~Client(void);

		std::string& getRecvBuf();
        std::string& getSendBuf();

		bool			tryBuildResponse(void);

	private:

		HttpRequest			_request;
		const ServerConfig&	_serverConfig;

		std::string		_parsePathFromRequest(void) const;
		std::string		_readFile(const std::string& filepath) const;
};

#endif
