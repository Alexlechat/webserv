#include "config/ConfigParser.hpp"
#include "server/EventLoop.hpp"
#include "logger/ConsoleLogger.hpp"
#include "logger/Logger.hpp"
#include "server/Client.hpp"
#include "server/Server.hpp"
#include "utils/Utils.hpp"
#include "http/Cgi.hpp"

#include <map>
#include <cerrno>
#include <cstdlib>
#include <csignal>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <exception>
#include <sys/poll.h>
#include <sys/wait.h>

volatile sig_atomic_t	g_server_running = 1;

static struct pollfd	make_pollfd(int fd, short events)
{
	struct pollfd	pfd;
	pfd.fd = fd;
	pfd.events = events;
	pfd.revents = 0;
	return pfd;
}

void	signal_handler(int sig)
{
	if (sig == SIGINT)
	{
		g_server_running = 0;
		write(STDOUT_FILENO, "\n", 1);
	}
	return ;
}

EventLoop::EventLoop(const std::string& configPath) : _logger()
{
	signal(SIGINT,  signal_handler);
	signal(SIGPIPE, SIG_IGN);

	try
	{
		ConfigParser	configParser(configPath);

		std::map<unsigned short, std::vector<ServerConfig> >	byPort;
		std::vector<unsigned short>								portOrder;

		std::vector<ServerConfig>::const_iterator	It;
		for (It = configParser.getServerConfigs().begin(); It != configParser.getServerConfigs().end(); It++)
		{
			if (byPort.find(It->port) == byPort.end())
				portOrder.push_back(It->port);
			byPort[It->port].push_back(*It);
		}

		for (size_t i = 0; i < portOrder.size(); ++i)
		{
			Server*	newServer = new Server(byPort[portOrder[i]]);
			_logger.addLogger(newServer->getServerErrorLogger());
			addServer(newServer);
			// Report the interface we actually bound, not a hardcoded one.
			const std::vector<ServerConfig>&	group = byPort[portOrder[i]];
			std::string	boundHost = group[0].host.specified && !group[0].host.value.empty()
								? group[0].host.value : std::string("0.0.0.0");
			LOG_INFO(ConsoleLogger::instance(),
				"Server listening on " + boundHost + ":" + Utils::toString(portOrder[i]));
		}
	}
	catch (const std::exception& e)
	{
		for (size_t j = 0; j < _servers.size(); ++j)
			delete _servers[j];
		_servers.clear();
		LOG_CRITICAL(ConsoleLogger::instance(), e.what());
		throw;
	}

	_logger.addLogger(ConsoleLogger::instance());
}

EventLoop::~EventLoop(void)
{
	std::vector<Server*>::iterator	serverIt;

	// A Client* can appear at more than one index (main socket + its
	// CGI pipes), but only the FD_CLIENT slot owns it -- deleting via
	// any other view would double-free.
	for (size_t i = 0; i < _fds.size(); ++i)
		if (_fdTypes[i] == FD_CLIENT)
			delete _clients[i];

	for (serverIt = _servers.begin(); serverIt != _servers.end(); serverIt++)
		delete *serverIt;
}

void	EventLoop::addServer(Server* server)
{
	server->setNonBlocking();
	_fds.push_back(make_pollfd(server->getSocketFd(), POLLIN));
	_fdTypes.push_back(FD_SERVER);
	_servers.push_back(server);
	_clients.push_back(NULL);
}

int	EventLoop::run(void)
{
	LOG_INFO(ConsoleLogger::instance(), "Server is running");
	while (g_server_running)
	{
		int	ready = poll(_fds.data(), (nfds_t)_fds.size(), POLL_TICK_MS);
		if (ready < 0)
		{
			if (errno == EINTR)
				continue;
			break;
		}

		_closeTimedOutClients();
		_checkCgiTimeouts();
		_reapZombies();

		if (ready == 0)
			continue;

		for (int i = (int)_fds.size() - 1; i >= 0; --i)
		{
			// A CGI pipe that already closed gets retired in place
			// (fd = -1); sweep it out now. Safe during a reverse walk.
			if (_fds[i].fd == -1)
			{
				_fds.erase(_fds.begin() + i);
				_fdTypes.erase(_fdTypes.begin() + i);
				_clients.erase(_clients.begin() + i);
				continue;
			}

			if (_fds[i].revents == 0)
				continue;

			switch (_fdTypes[i])
			{
				case FD_SERVER:
					_acceptNewClient(_getServerByFd(_fds[i].fd));
					break;

				case FD_CLIENT:
				{
					size_t	sizeBefore = _fds.size();

					if (_fds[i].revents & POLLIN)
						_handleRead(i);

					if (_fds.size() != sizeBefore)
						break;

					if (_fds[i].revents & POLLOUT)
						_handleWrite(i);

					if (_fds.size() != sizeBefore)
						break;

					// The peer hung up or the socket broke. poll() reports it
					// directly, which is how we drop dead connections without
					// ever inspecting errno after a send()/recv().
					if (_fds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
						_removeClient(i);
					break;
				}

				case FD_CGI_STDIN:
				{
					Client*	client = _clients[i];
					Cgi*	cgi = client->getCgi();

					cgi->handleWritable();
					if (!cgi->wantsWrite())
						_fds[i].fd = -1;
					if (cgi->isFinished())
						_completeCgi(client);
					break;
				}

				case FD_CGI_STDOUT:
				{
					Client*	client = _clients[i];
					Cgi*	cgi = client->getCgi();

					cgi->handleReadable();
					if (!cgi->wantsRead())
						_fds[i].fd = -1;
					if (cgi->isFinished())
						_completeCgi(client);
					break;
				}
			}
		}
	}

	return 0;
}

bool	EventLoop::_isServerFd(int fd) const
{
	for (size_t i = 0; i < _servers.size(); ++i)
		if (_servers[i]->getSocketFd() == fd) return true;
	return false;
}

Server*	EventLoop::_getServerByFd(int fd) const
{
	for (size_t i = 0; i < _servers.size(); ++i)
		if (_servers[i]->getSocketFd() == fd) return _servers[i];
	return NULL;
}

void	EventLoop::_acceptNewClient(Server* server)
{
	struct sockaddr_in	addr;
	socklen_t			len = sizeof(addr);
	int client_fd = accept(server->getSocketFd(), (struct sockaddr*)&addr, &len);

	if (client_fd < 0)
	{
		LOG_WARNING(server->getServerErrorLogger(), "accept() failure");
		return;
	}

	Client*	new_client = new Client(client_fd, server->getServerConfigs(), server->getServerAccessLogger());

	new_client->setSockAddr(addr);
	new_client->setNonBlocking();
	_fds.push_back(make_pollfd(client_fd, POLLIN));
	_fdTypes.push_back(FD_CLIENT);
	_clients.push_back(new_client);
}

void	EventLoop::_handleRead(int i)
{
	Client&	client = *_clients[i];
	char	buf[65536];
	ssize_t	n = recv(_fds[i].fd, buf, sizeof(buf) - 1, 0);

	if (n <= 0)
	{
		_removeClient(i);
		return;
	}

	Client::FeedResult	res = client.feed(buf, (size_t)n);

	if (res == Client::FEED_RESPONSE_READY)
	{
		_fds[i].events = POLLOUT;
	}
	else if (res == Client::FEED_CGI_STARTED)
	{
		// Nothing to read/write on the client socket while the CGI
		// runs -- the pipe fds are what poll() should watch now.
		_fds[i].events = 0;
		_registerCgi(&client);
	}
}

void	EventLoop::_handleWrite(int i)
{
	Client&		client = *_clients[i];
	std::string&	buf = client.getSendBuf();

	ssize_t n = send(_fds[i].fd, buf.c_str(), buf.size(), 0);

	if (n > 0)
	{
		buf.erase(0, n);
		client.touch();
	}

	if (!buf.empty())
	{
		// Short write, or the socket was not writable after all: leave
		// the rest queued and come back when poll() says so. errno is
		// never consulted here -- a peer that is really gone shows up as
		// POLLERR/POLLHUP, and a silent one is caught by the idle sweep.
		return;
	}

	client.logAccess();
	if (client.keepAlive())
	{
		client.resetForNextRequest();
		_fds[i].events = POLLIN;
	}
	else
		_removeClient(i);
}

void	EventLoop::_removeClient(int i)
{
	Client*	client = _clients[i];

	if (client->hasCgi())
	{
		Cgi*	cgi = client->getCgi();
		cgi->kill();
		_zombiePids.push_back(cgi->getPid());
		_retireCgiFds(client);
		_untrackCgiClient(client);
	}

	delete client;
	_fds.erase(_fds.begin() + i);
	_fdTypes.erase(_fdTypes.begin() + i);
	_clients.erase(_clients.begin() + i);
}

void	EventLoop::_closeTimedOutClients(void)
{
	time_t	now = std::time(NULL);

	for (int i = (int)_fds.size() - 1; i >= 0; --i)
	{
		if (_fdTypes[i] != FD_CLIENT)
			continue;

		if (now - _clients[i]->getLastActivity() >= CLIENT_IDLE_TIMEOUT_SECONDS)
		{
			LOG_WARNING(ConsoleLogger::instance(), "Closing idle/slow client connection (timeout)");
			_removeClient(i);
		}
	}
}

// --- CGI: same poll() array as everything else -----------------------

void	EventLoop::_registerCgi(Client* client)
{
	Cgi*	cgi = client->getCgi();

	if (cgi->wantsWrite())
	{
		_fds.push_back(make_pollfd(cgi->getStdinFd(), POLLOUT));
		_fdTypes.push_back(FD_CGI_STDIN);
		_clients.push_back(client);
	}
	if (cgi->wantsRead())
	{
		_fds.push_back(make_pollfd(cgi->getStdoutFd(), POLLIN));
		_fdTypes.push_back(FD_CGI_STDOUT);
		_clients.push_back(client);
	}

	_cgiClients.push_back(client);
}

void	EventLoop::_completeCgi(Client* client)
{
	Cgi*	cgi = client->getCgi();
	if (!cgi)
		return; // stdin+stdout both fired this tick; already handled

	// finishCgi() may reap the child itself while deciding whether the
	// script failed; it hands back only what is still owed a waitpid().
	pid_t	pending = client->finishCgi(); // builds the HttpResponse, deletes the Cgi

	if (pending > 0)
		_zombiePids.push_back(pending);
	_retireCgiFds(client);
	_activateClientWrite(client);
	_untrackCgiClient(client);
}

void	EventLoop::_checkCgiTimeouts(void)
{
	for (int i = (int)_cgiClients.size() - 1; i >= 0; --i)
	{
		Client*	client = _cgiClients[i];
		Cgi*	cgi = client->getCgi();

		if (!cgi || !cgi->isTimedOut())
			continue;

		LOG_WARNING(ConsoleLogger::instance(), "CGI script timed out, killing it");

		pid_t	pid = cgi->getPid();
		cgi->kill();
		client->abortCgi(Http::SERVICE_UNAVAILABLE);

		_zombiePids.push_back(pid);
		_retireCgiFds(client);
		_activateClientWrite(client);
		_cgiClients.erase(_cgiClients.begin() + i);
	}
}

void	EventLoop::_reapZombies(void)
{
	for (int i = (int)_zombiePids.size() - 1; i >= 0; --i)
	{
		int	status = 0;
		if (waitpid(_zombiePids[i], &status, WNOHANG) != 0) // never blocks
			_zombiePids.erase(_zombiePids.begin() + i);
	}
}

void	EventLoop::_retireCgiFds(Client* client)
{
	for (size_t i = 0; i < _fds.size(); ++i)
		if (_clients[i] == client && (_fdTypes[i] == FD_CGI_STDIN || _fdTypes[i] == FD_CGI_STDOUT))
			_fds[i].fd = -1;
}

void	EventLoop::_activateClientWrite(Client* client)
{
	for (size_t i = 0; i < _fds.size(); ++i)
	{
		if (_fdTypes[i] == FD_CLIENT && _clients[i] == client)
		{
			_fds[i].events = POLLOUT;
			break;
		}
	}
}

void	EventLoop::_untrackCgiClient(Client* client)
{
	for (size_t k = 0; k < _cgiClients.size(); ++k)
	{
		if (_cgiClients[k] == client)
		{
			_cgiClients.erase(_cgiClients.begin() + k);
			break;
		}
	}
}
