#include "config/ConfigParser.hpp"
#include "server/EventLoop.hpp"
#include "server/Server.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <exception>

static void	set_nonblocking(int fd)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
		std::cerr << "fcntl(): failure on fd=" << fd << "\n";
}

static struct pollfd	make_pollfd(int fd, short	events)
{
	struct pollfd	pfd;

	pfd.fd = fd;
	pfd.events = events;
	pfd.revents = 0;

	return pfd;
}

EventLoop::EventLoop(void) {}

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
	set_nonblocking(server->getServerFd());
	_fds.push_back(make_pollfd(server->getServerFd(), POLLIN));
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
			std::cout << "New Server added : " << It->server_name << ":" << It->port << std::endl;
			addServer(newServer);
		}
	}
	catch (const std::exception& e) { std::cerr << e.what() << std::endl; return 1; }

	while (true)
	{
		int	ready = poll(_fds.data(), (nfds_t)_fds.size(), -1);
		if (ready < 0)
		{
			std::cerr << "poll(): failure" << std::endl;
			break ;
		}

		for (int i = (int)_fds.size() - 1; i >= 0; --i)
		{
			if (_fds[i].revents == 0)
				continue ;
			if (_isServerFd(_fds[i].fd))
			{
				_acceptNewClient(_fds[i].fd);
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
		if (_servers[i]->getServerFd() == fd) return true;

	return false;
}

Server*	EventLoop::_getServerByFd(int fd) const
{
	for (size_t i = 0; i < _servers.size(); ++i)
		if (_servers[i]->getServerFd() == fd) return _servers[i];

	return NULL;
}

void	EventLoop::_acceptNewClient(int server_fd)
{
	struct sockaddr_in	addr;
	socklen_t			len = sizeof(addr);
	int client_fd = accept(server_fd, (struct sockaddr*)&addr, &len);

	if (client_fd < 0)
	{
		std::cerr << "accept(): failure\n";
		return;
	}
 
	set_nonblocking(client_fd);
 
	_fds.push_back(make_pollfd(client_fd, POLLIN));
	_clients.push_back(new Client(client_fd, _getServerByFd(server_fd)->getServerConfig()));
 
	std::cout << "New client fd=" << client_fd << "\n";
}
 
void	EventLoop::_handleRead(int i)
{
	Client&	client = *_clients[i];
	char	buf[4096];
	ssize_t	n = recv(_fds[i].fd, buf, sizeof(buf) - 1, 0);
 
	if (n <= 0)
	{
		// n == 0 → client closed connection cleanly
		// n < 0  → recv error
		_removeClient(i);
		return;
	}
 
	buf[n] = '\0';
	client.getRecvBuf() += buf;
 
	if (client.tryBuildResponse())
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
		std::cout << "Response sent, closing fd=" << _fds[i].fd << "\n";
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
