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
		// What happened after feeding fresh bytes into the parser:
		//  - FEED_CONTINUE:      need more bytes.
		//  - FEED_CGI_STARTED:   request routes to CGI; a Cgi has been
		//                        start()-ed and is now owned by this
		//                        Client. EventLoop must register its
		//                        pipe fds with poll() and pause this
		//                        client's own socket until it finishes.
		//  - FEED_RESPONSE_READY: _send_buf is filled in, switch this
		//                        client's socket to POLLOUT.
		enum FeedResult { FEED_CONTINUE, FEED_CGI_STARTED, FEED_RESPONSE_READY };

		Client(int fd, const std::vector<ServerConfig>& serverConfigs, const FileLogger& accessLogger);
		~Client(void);

		FeedResult			feed(const char* buf, size_t n);
		void				logAccess(void) const;

		time_t				getLastActivity(void) const;
		// Mark progress so a slow but healthy transfer is not mistaken
		// for an idle connection by the timeout sweep.
		void				touch(void);
		bool				isAwaitingResponse(void) const;
		bool				keepAlive(void) const;
		void				resetForNextRequest(void);

		bool				hasCgi(void) const;
		Cgi*				getCgi(void) const;

		// Called by EventLoop once the owned Cgi has finished normally
		// (stdout hit EOF): builds the HttpResponse from its output,
		// deletes it, and fills _send_buf. Returns the child pid still
		// waiting to be reaped, or -1 if the Cgi already collected it.
		pid_t				finishCgi(void);
		// Called by EventLoop when the owned Cgi has to be aborted
		// (timeout; already killed by the caller): builds an error
		// response instead and deletes the Cgi.
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
