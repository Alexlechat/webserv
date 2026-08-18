#ifndef Server_HPP
# define Server_HPP

# include "config/ServerConfig.hpp"
# include "socket/SocketServer.hpp"

# include "logger/FileLogger.hpp"

# include <vector>

# define LISTEN_MAX 100

class	Server : public SocketServer
{
	public:
		Server(const std::vector<ServerConfig>& serverConfigs);
		~Server(void);

		const FileLogger&					getServerErrorLogger(void) const;
		const FileLogger&					getServerAccessLogger(void) const;
		const std::vector<ServerConfig>&	getServerConfigs(void) const;

	private:
		std::vector<ServerConfig>			_serverConfigs;
		FileLogger*							_fileErrorLogger;
		FileLogger*							_fileAccessLogger;

		Server(const Server&);
		Server&	operator=(const Server&);
};

#endif
