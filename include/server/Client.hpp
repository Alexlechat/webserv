#ifndef Client_HPP
# define Client_HPP

# include "http/HttpResponse.hpp"
# include "logger/FileLogger.hpp"
# include "socket/SocketClient.hpp"
# include "config/ServerConfig.hpp"
# include "http/HttpRequest.hpp"

# include <ctime>
# include <vector>
# include <sys/types.h>

class	Cgi;

class	Client : public SocketClient
{
	public:
		enum FeedResult { FEED_CONTINUE, FEED_CGI_STARTED, FEED_RESPONSE_READY };

		Client(int fd, const std::vector<ServerConfig>& serverConfigs, const FileLogger& accessLogger);
		~Client(void);

		FeedResult			feed(const char* buf, size_t n);
		void				logAccess(void) const;

		time_t				getLastActivity(void) const;
		void				touch(void);
		bool				isAwaitingResponse(void) const;
		bool				keepAlive(void) const;
		void				resetForNextRequest(void);

		bool				hasCgi(void) const;
		Cgi*				getCgi(void) const;

		pid_t				finishCgi(void);
		void				abortCgi(Http::StatusCode code);

	private:
		std::string					_ip;
		std::string					_bytesBodySent;
		Http::StatusCode			_status;
		HttpRequest					_request;
		HttpResponse				_response;
		const FileLogger&			_accessLogger;
		const std::vector<ServerConfig>&	_serverConfigs;
		time_t						_lastActivity;
		bool						_awaitingResponse;
		bool						_keepAlive;
		bool						_forceClose;
		Cgi*						_cgi;

		const ServerConfig&	_selectConfig(void) const;
		bool				_wantsKeepAlive(void) const;

		FeedResult	_onRequestComplete(void);
		void		_onParseError(Http::StatusCode code);
		void		_captureIp(void);
		void		_finalizeResponse(void);
};

#endif
