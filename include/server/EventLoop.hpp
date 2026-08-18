#ifndef EventLoop_HPP
# define EventLoop_HPP

# include "server/Server.hpp"
# include "server/Client.hpp"

# include "logger/MultiLogger.hpp"

# include <vector>
# include <sys/poll.h>
# include <sys/types.h>

# define POLL_TICK_MS 1000
# define CLIENT_IDLE_TIMEOUT_SECONDS 60

class	EventLoop
{
	public:
		EventLoop(const std::string& configPath);
		~EventLoop(void);

		int		run(void);
		void	addServer(Server* server);

	private:
		// Every entry in _fds has a matching entry (same index) in
		// _fdTypes and _clients: this is what lets a single poll()
		// call multiplex listening sockets, client sockets, AND CGI
		// pipes together, instead of CGI running its own private
		// blocking poll() loop.
		enum FdType { FD_SERVER, FD_CLIENT, FD_CGI_STDIN, FD_CGI_STDOUT };

		MultiLogger					_logger;

		std::vector<struct pollfd>	_fds;
		std::vector<FdType>			_fdTypes;
		std::vector<Client*>		_clients;   // owning Client* per slot; NULL for FD_SERVER
		std::vector<Server*>		_servers;

		std::vector<Client*>		_cgiClients; // clients with a running CGI (timeout scan)
		std::vector<pid_t>			_zombiePids; // pids awaiting a non-blocking reap

		bool	_isServerFd(int fd) const;
		Server*	_getServerByFd(int fd) const;

		void	_acceptNewClient(Server* server);
		void	_handleRead(int i);
		void	_handleWrite(int i);
		void	_removeClient(int i);
		void	_closeTimedOutClients(void);

		void	_registerCgi(Client* client);
		void	_completeCgi(Client* client);
		void	_checkCgiTimeouts(void);
		void	_reapZombies(void);
		void	_retireCgiFds(Client* client);
		void	_activateClientWrite(Client* client);
		void	_untrackCgiClient(Client* client);
};

#endif
