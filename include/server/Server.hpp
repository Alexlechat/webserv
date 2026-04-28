#ifndef Server_HPP
# define Server_HPP

# include <string>
# include <exception>
# include <netinet/in.h>

#include "socket/SocketServer.hpp"

# define DEFAULT_PORT 8080
# define LISTEN_MAX 100

class	Server : public SocketServer
{
	public:
		Server(int port);
		~Server(void);

		int	getServerFd(void) const;
};

#endif  // Server_HPP
