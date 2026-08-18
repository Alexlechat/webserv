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

		bool	start(void);

		pid_t	getPid(void) const;

		int		getStdinFd(void) const;
		int		getStdoutFd(void) const;

		bool	wantsWrite(void) const;
		bool	wantsRead(void) const;

		void	handleWritable(void);
		void	handleReadable(void);

		bool	isFinished(void) const;
		bool	isTimedOut(void) const;

		void	kill(void);

		bool	failed(void);

		pid_t	pendingPid(void) const;

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
		bool		_reaped;
		bool		_exitedOk;

		void						_collectExitStatus(void);
		std::vector<std::string>	_buildEnv(void) const;
		HttpResponse				_parseOutput(const std::string& output) const;

		Cgi(const Cgi&);
		Cgi&	operator=(const Cgi&);
};

#endif
