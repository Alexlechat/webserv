#ifndef Cgi_HPP
# define Cgi_HPP

# include "http/HttpRequest.hpp"
# include "http/HttpResponse.hpp"
# include "config/ServerConfig.hpp"
# include "config/LocationConfig.hpp"

# include <string>
# include <vector>
# include <ctime>
# include <sys/types.h>

# define CGI_TIMEOUT_SECONDS 5

// Non-blocking CGI runner.
//
// This class never blocks and never runs its own poll() loop: it only
// exposes the stdin/stdout pipe fds it wants monitored, plus step
// functions (handleWritable / handleReadable) that the caller invokes
// once EventLoop's single, global poll() reports those fds are ready.
// This is what lets a CGI script run without ever stalling any other
// client connected to the server.
class	Cgi
{
	public:
		Cgi(const HttpRequest& request,
			const ServerConfig& config,
			const LocationConfig& location,
			const std::string& scriptPath,
			const std::string& pathInfo,
			const std::string& interpreter);
		~Cgi(void);

		// Forks + execve's the interpreter and wires up non-blocking
		// pipes. Returns false on fork()/pipe() failure; nothing to
		// clean up in that case (the caller should just report 500).
		bool	start(void);

		pid_t	getPid(void) const;

		// -1 once that side has been closed (nothing left to do there).
		int		getStdinFd(void) const;
		int		getStdoutFd(void) const;

		bool	wantsWrite(void) const;
		bool	wantsRead(void) const;

		// Call only once poll() reports the corresponding fd is ready.
		void	handleWritable(void);
		void	handleReadable(void);

		bool	isFinished(void) const;  // stdout hit EOF: ready to build the response
		bool	isTimedOut(void) const;

		void	kill(void); // SIGKILL the child (timeout / early client disconnect)

		HttpResponse	buildResponse(void) const;

	private:
		const HttpRequest&		_request;
		const ServerConfig&		_config;
		const LocationConfig&	_location;
		std::string				_scriptPath;
		std::string				_pathInfo;
		std::string				_interpreter;

		pid_t		_pid;
		int			_inFd;
		int			_outFd;
		std::string	_toWrite;
		size_t		_written;
		std::string	_output;
		time_t		_deadline;

		std::vector<std::string>	_buildEnv(void) const;
		HttpResponse				_parseOutput(const std::string& output) const;

		Cgi(const Cgi&);
		Cgi&	operator=(const Cgi&);
};

#endif  // Cgi_HPP
