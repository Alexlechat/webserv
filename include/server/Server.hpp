#ifndef Server_HPP
# define Server_HPP

# include "config/ServerConfig.hpp"

# include <string>
# include <exception>
# include <netinet/in.h>

# define LISTEN_MAX 100

class	Server
{
	public:
		Server(ServerConfig serverConfig);
		~Server(void);

		int					getServerFd(void) const;
		const ServerConfig&	getServerConfig(void) const;

		class	ServerException : public std::exception
		{
			public:
				ServerException(const std::string& msg) : _msg(msg) {}
				virtual const char*	what() const throw() { return _msg.c_str(); }
				virtual ~ServerException() throw() {}

			private:
				std::string _msg;
		};

	private:
		int					_fd;
		struct sockaddr_in	_addr;
		ServerConfig		_serverConfig;

		void	_initSocket(int port);
		void	_bindSocket(void) const;
		void	_listenSocket(int maxConnections) const;
};

#endif  // Server_HPP
