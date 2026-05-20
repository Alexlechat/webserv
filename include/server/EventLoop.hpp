#ifndef EventLoop_HPP
# define EventLoop_HPP

# include "server/Server.hpp"
# include "server/Client.hpp"

# include "logger/MultiLogger.hpp"

# include <vector>
# include <sys/poll.h>

class	EventLoop
{
	public:
		EventLoop(void);
		~EventLoop(void);

		int		run(void);
		void	addServer(Server* server);

	private:
		MultiLogger					_logger;

		std::vector<struct pollfd>	_fds;
		std::vector<Server*>		_servers;
		std::vector<Client*>		_clients;

		bool	_isServerFd(int fd) const;
		Server*	_getServerByFd(int fd) const;

		void	_acceptNewClient(Server* server);
		void	_handleRead(int i);
		void	_handleWrite(int i);
		void	_removeClient(int i);
};

#endif  // EventLoop_HPP
