#ifndef Client_HPP
# define Client_HPP

# include "logger/FileLogger.hpp"
# include "socket/SocketClient.hpp"
# include "config/ServerConfig.hpp"
# include "http/HttpRequest.hpp"


class	Client : public SocketClient
{
	public:
		Client(int fd, const ServerConfig& serverConfig, const FileLogger& accessLogger);
		~Client(void);

		bool				feed(const char* buf, size_t n);
		void				logAccess(void) const;

	private:
		std::string			_ip;
		std::string			_status;
		std::string			_bytesBodySent;
		HttpRequest			_request;
		const FileLogger&	_accessLogger;
		const ServerConfig&	_serverConfig;

		void	_onRequestComplete(void);
		void	_onParseError(Http::StatusCode code);
};

#endif
