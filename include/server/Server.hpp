#ifndef Server_HPP
# define Server_HPP

# include "config/ServerConfig.hpp"
# include "socket/SocketServer.hpp"

# include "logger/FileLogger.hpp"
# include "logger/ConsoleLogger.hpp"

# define LISTEN_MAX 100

class	Server : public SocketServer
{
	public:
		Server(ServerConfig serverConfig);
		~Server(void);

		const ServerConfig&	getServerConfig(void) const;

	private:
		ServerConfig		_serverConfig;

		FileLogger			_file;
		ConsoleLogger		_console;
};

#endif  // Server_HPP
