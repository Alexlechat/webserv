#ifndef Client_HPP
# define Client_HPP

# include "server/Server.hpp"
# include "logger/FileLogger.hpp"
# include "socket/SocketClient.hpp"
# include "http/HttpRequest.hpp"
# include "http/HttpResponse.hpp"


class	Client : public SocketClient
{
	public:
		Client(int fd, const Server& server, const FileLogger& accessLogger);
		~Client(void);

		bool				feed(const char* buf, size_t n);
		void				logAccess(void);

	private:
		std::string			_ip;
		std::string			_bytesBodySent;
		Http::StatusCode	_status;
		HttpRequest			_request;
		HttpResponse		_response;
		const Server&		_server;
		const FileLogger&	_accessLogger;

		void	_onRequestComplete(void);
		void	_onParseError(Http::StatusCode code);
};

#endif
