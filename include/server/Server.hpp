#ifndef Server_HPP
# define Server_HPP

# include "config/ServerConfig.hpp"
# include "socket/SocketServer.hpp"

# include <string>
# include <exception>
# include <netinet/in.h>

# define LISTEN_MAX 100

class	Server : public SocketServer
{
	public:
		Server(ServerConfig serverConfig);
		~Server(void);

		const ServerConfig&	getServerConfig(void) const;

	private:
		ServerConfig		_serverConfig;
};

#endif  // Server_HPP
