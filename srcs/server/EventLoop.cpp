#include "config/ConfigParser.hpp"
#include "server/EventLoop.hpp"
#include "logger/ConsoleLogger.hpp"
#include "logger/Logger.hpp"
#include "server/Client.hpp"
#include "server/Server.hpp"
#include "utils/Utils.hpp"

#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <exception>
#include <sys/poll.h>

static struct pollfd	make_pollfd(int fd, short events)
{
	struct pollfd	pfd;

	pfd.fd = fd;
	pfd.events = events;
	pfd.revents = 0;

	return pfd;
}

EventLoop::EventLoop(void) : _logger()
{
	try
	{
		ConfigParser	configParser("./www/config");

		std::vector<ServerConfig>::const_iterator	It;
		for (It = configParser.getServerConfigs().begin(); It != configParser.getServerConfigs().end(); It++)
		{
			Server*	newServer = new Server(*It);
			_logger.addLogger(newServer->getServerErrorLogger());
			addServer(newServer);
			LOG_INFO(ConsoleLogger::instance(), "New Server added on port " + Utils::toString(It->port));
		}
	}
	catch (const std::exception& e)
	{
		LOG_CRITICAL(ConsoleLogger::instance(), e.what());
		exit(1);
	}

	_logger.addLogger(ConsoleLogger::instance());
}

EventLoop::~EventLoop(void)
{
	std::vector<Server*>::iterator	serverIt;
	std::vector<Client*>::iterator	clientIt;

	for (serverIt = _servers.begin(); serverIt != _servers.end(); serverIt++)
		delete *serverIt;

	for (clientIt = _clients.begin(); clientIt != _clients.end(); clientIt++)
		delete *clientIt;
}

void	EventLoop::addServer(Server* server)
{
	server->setNonBlocking();
	_fds.push_back(make_pollfd(server->getSocketFd(), POLLIN));
	_servers.push_back(server);
	_clients.push_back(NULL);
}

int	EventLoop::run(void)
{
	LOG_INFO(ConsoleLogger::instance(), "Starting poll loop");
	while (true)
	{
		int	ready = poll(_fds.data(), (nfds_t)_fds.size(), -1);
		if (ready < 0) { break; }

		for (int i = (int)_fds.size() - 1; i >= 0; --i)
		{
			if (_fds[i].revents == 0)
				continue;
			if (_isServerFd(_fds[i].fd))
			{
				_acceptNewClient(_getServerByFd(_fds[i].fd));
				continue;
			}
			
			if (_fds[i].revents & POLLIN)
				_handleRead(i);
			if (_fds[i].revents & POLLOUT)
				_handleWrite(i);
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

	Client*	new_client = new Client(client_fd, server->getServerConfig(), server->getServerAccessLogger());

	new_client->setSockAddr(addr);
	new_client->setNonBlocking();
	_fds.push_back(make_pollfd(client_fd, POLLIN));
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

	if (client.feed(buf, (size_t)n))
		_fds[i].events = POLLOUT;
}

void	EventLoop::_handleWrite(int i)
{
	Client&		client = *_clients[i];
	std::string&	buf = client.getSendBuf();

	ssize_t n = send(_fds[i].fd, buf.c_str(), buf.size(), 0);
 
	if (n > 0) buf.erase(0, n);
 
	if (buf.empty() || n <= 0)
	{
		client.logAccess();
		_removeClient(i);
	}
}

void	EventLoop::_removeClient(int i)
{
	close(_fds[i].fd);
	delete _clients[i];
	_fds.erase(_fds.begin() + i);
	_clients.erase(_clients.begin() + i);
}
