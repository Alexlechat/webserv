#ifndef Server_HPP
# define Server_HPP

# include "config/ServerConfig.hpp"
# include "socket/SocketServer.hpp"

# include "logger/FileLogger.hpp"

# include <vector>

# define LISTEN_MAX 100

// One Server = one bound/listening socket. Several "server {}" blocks in
// the config file can share the same "listen" port (classic nginx-style
// virtual hosting) -- they must NOT each open their own socket on that
// port (that would simply fail to bind a second time). So a Server owns
// every ServerConfig that listens on its port, and the actual config
// used to answer a given request is chosen from the Host header once
// the request has been parsed (see Client::_selectConfig).
class	Server : public SocketServer
{
	public:
		Server(const std::vector<ServerConfig>& serverConfigs);
		~Server(void);

		const FileLogger&					getServerErrorLogger(void) const;
		const FileLogger&					getServerAccessLogger(void) const;
		const std::vector<ServerConfig>&	getServerConfigs(void) const;

	private:
		std::vector<ServerConfig>			_serverConfigs;
		FileLogger*							_fileErrorLogger;
		FileLogger*							_fileAccessLogger;

		Server(const Server&);
		Server&	operator=(const Server&);
};

#endif  // Server_HPP
