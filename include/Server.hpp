#ifndef Server_HPP
# define Server_HPP

# include <string>
# include <exception>
# include <netinet/in.h>

# define DEFAULT_PORT 8080
# define LISTEN_MAX 100

class	Server
{
	public:
		Server(int port);
		~Server(void);

		int	getServerFd(void) const;

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

		void	_initSocket(int port);
		void	_bindSocket(void) const;
		void	_listenSocket(int maxConnections) const;
};

#endif  // Server_HPP
