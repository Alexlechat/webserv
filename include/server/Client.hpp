#ifndef Client_HPP
# define Client_HPP

# include "logger/FileLogger.hpp"
# include "socket/SocketClient.hpp"
# include "config/ServerConfig.hpp"
# include "request/HttpRequest.hpp"


class	Client : public SocketClient
{
	public:
		Client(int fd, const ServerConfig& serverConfig, const FileLogger& accessLogger);
		~Client(void);

		bool				tryBuildResponse(void);
		void				logAccess(void) const;

	private:
		HttpRequest			_request;
		const FileLogger&	_accessLogger;
		const ServerConfig&	_serverConfig;
};

#endif
