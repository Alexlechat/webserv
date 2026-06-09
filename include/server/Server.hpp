#ifndef Server_HPP
# define Server_HPP

# include "config/ServerConfig.hpp"
# include "socket/SocketServer.hpp"

# include "logger/FileLogger.hpp"

# define LISTEN_MAX 100

class	Server : public SocketServer
{
	public:
		Server(ServerConfig serverConfig);
		~Server(void);

		const FileLogger&	getServerErrorLogger(void) const;
		const FileLogger&	getServerAccessLogger(void) const;
		const ServerConfig&	getServerConfig(void) const;

	private:
		ServerConfig		_serverConfig;
		FileLogger*			_fileErrorLogger;
		FileLogger*			_fileAccessLogger;
};

#endif  // Server_HPP
