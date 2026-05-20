#include "config/ConfigParser.hpp"
#include "server/EventLoop.hpp"
#include "logger/ConsoleLogger.hpp"
#include "logger/Logger.hpp"
#include "server/Client.hpp"
#include "server/Server.hpp"

#include "utils/Utils.hpp"

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
	_logger.addLogger(&ConsoleLogger::instance());
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
	try
	{
		ConfigParser	configParser("./www/config");

		std::vector<ServerConfig>::const_iterator	It;
		for (It = configParser.getServerConfigs().begin(); It != configParser.getServerConfigs().end(); It++)
		{
			Server*	newServer = new Server(*It);
			LOG_FD_INFO(_logger, newServer, "New Server added on port " + Utils::toString(It->port));
			addServer(newServer);
		}
	}
	catch (const std::exception& e)
	{
		LOG_ERROR(_logger, e.what());
		return 1;
	}

	while (true)
	{
		int	ready = poll(_fds.data(), (nfds_t)_fds.size(), -1);
		if (ready < 0)
		{
			LOG_ERROR(_logger, "poll() failure");
			break ;
		}

		for (int i = (int)_fds.size() - 1; i >= 0; --i)
		{
			if (_fds[i].revents == 0)
				continue ;
			if (_isServerFd(_fds[i].fd))
			{
				_acceptNewClient(_getServerByFd(_fds[i].fd));
				continue ;
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
		LOG_FD_ERROR(_logger, server, "accept() failure");
		return;
	}

	Client*	new_client = new Client(client_fd, server->getServerConfig());

	new_client->setNonBlocking();
	_fds.push_back(make_pollfd(client_fd, POLLIN));
	_clients.push_back(new_client);

	LOG_FD_INFO(_logger, new_client, "New client accepted");
}

void	EventLoop::_handleRead(int i)
{
	Client&	client = *_clients[i];
	char	buf[4096];

	LOG_FD_INFO(_logger, _clients[i], "Reading data from client...");
	ssize_t	n = recv(_fds[i].fd, buf, sizeof(buf) - 1, 0);

	if (n <= 0)
	{
		LOG_FD_WARNING(_logger, _clients[i], "No data received, closing connection");
		_removeClient(i);
		return;
	}

	buf[n] = '\0';
	client.getRecvBuf() += buf;
 
	if (client.tryBuildResponse())
	{
		LOG_FD_INFO(_logger, _clients[i], "All data received");
		_fds[i].events = POLLOUT;
	}
}

void	EventLoop::_handleWrite(int i)
{
	Client&		client = *_clients[i];
	std::string&	buf = client.getSendBuf();

	LOG_FD_INFO(_logger, _clients[i], "Sending data to client...");
	ssize_t n = send(_fds[i].fd, buf.c_str(), buf.size(), 0);
 
	if (n > 0) buf.erase(0, n);
 
	if (buf.empty() || n <= 0)
	{
		LOG_FD_INFO(_logger, _clients[i], "All data sended");
		_removeClient(i);
	}
}

void	EventLoop::_removeClient(int i)
{
	LOG_FD_INFO(_logger, _clients[i], "Closing connection");
	close(_fds[i].fd);
	delete _clients[i];
	_fds.erase(_fds.begin() + i);
	_clients.erase(_clients.begin() + i);
}
