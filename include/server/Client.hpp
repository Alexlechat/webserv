#ifndef Client_HPP
# define Client_HPP

# include "config/ServerConfig.hpp"
# include "request/HttpRequest.hpp"
# include "socket/SocketClient.hpp"

# include <string>

class	Client : public SocketClient
{
	public:
		Client(int fd, const ServerConfig& serverConfig);
		~Client(void);

		bool	tryBuildResponse(void);

	private:
		HttpRequest			_request;
		const ServerConfig&	_serverConfig;
};

#endif
