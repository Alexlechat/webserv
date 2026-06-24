#ifndef Client_HPP
# define Client_HPP

# include "http/HttpResponse.hpp"
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
		std::string			_bytesBodySent;
		Http::StatusCode	_status;
		HttpRequest			_request;
		HttpResponse		_response;
		const FileLogger&	_accessLogger;
		const ServerConfig&	_serverConfig;

		void	_onRequestComplete(void);
		void	_onParseError(Http::StatusCode code);
};

#endif
