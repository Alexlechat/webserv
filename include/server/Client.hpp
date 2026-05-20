#ifndef Client_HPP
# define Client_HPP

# include "socket/SocketClient.hpp"
# include "config/ServerConfig.hpp"
# include "request/HttpRequest.hpp"


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
