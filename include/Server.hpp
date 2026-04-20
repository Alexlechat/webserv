#ifndef Server_HPP
# define Server_HPP

# include <string>
# include <netinet/in.h>

# define DEFAULT_PORT 8080
# define LISTEN_MAX 100

class	Server
{
	public:
		Server(void);
		~Server(void);
		Server(const Server& src);
		Server&	operator=(const Server& rhs);

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

		void	initSocket(int port);
		void	bindSocket(void) const;
		void	listenSocket(int maxConnections) const;
};

#endif  // Server_HPP
