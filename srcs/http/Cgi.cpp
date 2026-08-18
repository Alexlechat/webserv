#include "http/Cgi.hpp"
#include "utils/Utils.hpp"

#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <signal.h>
#include <sstream>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

Cgi::Cgi(const HttpRequest& request,
		const ServerConfig& config,
		const LocationConfig& location,
		const std::string& scriptPath,
		const std::string& pathInfo,
		const std::string& interpreter)
	: _request(request), _config(config), _location(location),
	  _scriptPath(scriptPath), _pathInfo(pathInfo), _interpreter(interpreter),
	  _pid(-1), _inFd(-1), _outFd(-1), _written(0), _deadline(0),
	  _reaped(false), _exitedOk(true)
{}

Cgi::~Cgi(void)
{
	if (_inFd != -1)
		close(_inFd);
	if (_outFd != -1)
		close(_outFd);
}

std::vector<std::string>	Cgi::_buildEnv(void) const
{
	std::vector<std::string>	env;

	env.push_back("REQUEST_METHOD=" + _request.method);
	env.push_back("QUERY_STRING=" + _request.query);
	env.push_back("PATH_INFO=" + _pathInfo);
	env.push_back("SCRIPT_NAME=" + _request.path);
	env.push_back("SCRIPT_FILENAME=" + _scriptPath);
	env.push_back("SERVER_PROTOCOL=" + (_request.version.empty() ? "HTTP/1.1" : _request.version));
	env.push_back("SERVER_NAME=" + (_config.server_name->empty() ? std::string("localhost") : _config.server_name));
	env.push_back("SERVER_PORT=" + Utils::toString(_config.port));
	env.push_back("GATEWAY_INTERFACE=CGI/1.1");
	env.push_back("SERVER_SOFTWARE=webserv/1.0");
	env.push_back("REDIRECT_STATUS=200");

	std::map<std::string, std::string>::const_iterator	cl = _request.headers.find("content-length");
	env.push_back("CONTENT_LENGTH=" + (cl != _request.headers.end() ? cl->second : Utils::toString(_request.body.size())));

	std::map<std::string, std::string>::const_iterator	ct = _request.headers.find("content-type");
	if (ct != _request.headers.end())
		env.push_back("CONTENT_TYPE=" + ct->second);

	for (std::map<std::string, std::string>::const_iterator it = _request.headers.begin();
		 it != _request.headers.end(); ++it)
	{
		if (it->first == "content-length" || it->first == "content-type")
			continue;

		std::string	key = "HTTP_" + Utils::strToUpper(it->first);
		for (size_t i = 0; i < key.size(); ++i)
			if (key[i] == '-')
				key[i] = '_';

		env.push_back(key + "=" + it->second);
	}

	(void)_location;
	return env;
}

bool	Cgi::start(void)
{
	int	inPipe[2];
	int	outPipe[2];

	if (pipe(inPipe) < 0)
		return false;
	if (pipe(outPipe) < 0)
	{
		close(inPipe[0]); close(inPipe[1]);
		return false;
	}

	pid_t	pid = fork();
	if (pid < 0)
	{
		close(inPipe[0]); close(inPipe[1]);
		close(outPipe[0]); close(outPipe[1]);
		return false;
	}

	if (pid == 0)
	{
		dup2(inPipe[0], STDIN_FILENO);
		dup2(outPipe[1], STDOUT_FILENO);
		close(inPipe[0]); close(inPipe[1]);
		close(outPipe[0]); close(outPipe[1]);

		for (int fd = STDERR_FILENO + 1; fd < 1024; ++fd)
			close(fd);

		std::string	dir = ".";
		std::string	base = _scriptPath;
		size_t		slash = _scriptPath.find_last_of('/');
		if (slash != std::string::npos)
		{
			dir = _scriptPath.substr(0, slash);
			base = _scriptPath.substr(slash + 1);
		}
		if (chdir(dir.c_str()) != 0)
			_exit(1);

		std::vector<std::string>	envStrings = _buildEnv();
		std::vector<char*>			envp;
		for (size_t i = 0; i < envStrings.size(); ++i)
			envp.push_back(const_cast<char*>(envStrings[i].c_str()));
		envp.push_back(NULL);

		char*	argv[3];
		argv[0] = const_cast<char*>(_interpreter.c_str());
		argv[1] = const_cast<char*>(base.c_str());
		argv[2] = NULL;

		execve(_interpreter.c_str(), argv, &envp[0]);
		_exit(1);
	}

	_pid = pid;
	close(inPipe[0]);
	close(outPipe[1]);
	_inFd = inPipe[1];
	_outFd = outPipe[0];

	fcntl(_inFd, F_SETFL, O_NONBLOCK);
	fcntl(_outFd, F_SETFL, O_NONBLOCK);

	_toWrite = _request.body;
	_written = 0;

	if (_toWrite.empty())
	{
		close(_inFd);
		_inFd = -1;
	}

	_deadline = std::time(NULL) + CGI_TIMEOUT_SECONDS;
	return true;
}

pid_t	Cgi::getPid(void) const      { return _pid; }
int		Cgi::getStdinFd(void) const  { return _inFd; }
int		Cgi::getStdoutFd(void) const { return _outFd; }
bool	Cgi::wantsWrite(void) const  { return _inFd != -1; }
bool	Cgi::wantsRead(void) const   { return _outFd != -1; }
bool	Cgi::isFinished(void) const  { return _outFd == -1; }
bool	Cgi::isTimedOut(void) const  { return std::time(NULL) >= _deadline; }

void	Cgi::handleWritable(void)
{
	if (_inFd == -1)
		return;

	ssize_t	n = write(_inFd, _toWrite.c_str() + _written, _toWrite.size() - _written);
	if (n > 0)
		_written += (size_t)n;

	if (_written >= _toWrite.size() || n <= 0)
	{
		close(_inFd);
		_inFd = -1;
	}
}

void	Cgi::handleReadable(void)
{
	if (_outFd == -1)
		return;

	char	buf[65536];
	ssize_t	n = read(_outFd, buf, sizeof(buf));

	if (n > 0)
	{
		_output.append(buf, (size_t)n);
		return;
	}

	close(_outFd);
	_outFd = -1;
}

void	Cgi::kill(void)
{
	if (_pid > 0)
		::kill(_pid, SIGKILL);
}

void	Cgi::_collectExitStatus(void)
{
	if (_reaped || _pid <= 0)
		return;

	int	status = 0;
	if (waitpid(_pid, &status, WNOHANG) != _pid)
		return;

	_reaped = true;
	_exitedOk = WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

bool	Cgi::failed(void)
{
	_collectExitStatus();

	if (_output.empty())
		return true;

	return _reaped && !_exitedOk;
}

pid_t	Cgi::pendingPid(void) const { return _reaped ? -1 : _pid; }

HttpResponse	Cgi::buildResponse(void) const
{
	return _parseOutput(_output);
}

HttpResponse	Cgi::_parseOutput(const std::string& output) const
{
	size_t	headerEnd = output.find("\r\n\r\n");
	size_t	sepLen = 4;
	if (headerEnd == std::string::npos)
	{
		headerEnd = output.find("\n\n");
		sepLen = 2;
	}

	if (headerEnd == std::string::npos)
	{
		HttpResponse	resp(Http::OK);
		resp.setBody(output, "text/html");
		return resp;
	}

	std::string	headerBlock = output.substr(0, headerEnd);
	std::string	body = output.substr(headerEnd + sepLen);

	HttpResponse		resp(Http::OK);
	std::string			contentType = "text/html";
	std::istringstream	iss(headerBlock);
	std::string			line;

	while (std::getline(iss, line))
	{
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		if (line.empty())
			continue;

		size_t	colon = line.find(':');
		if (colon == std::string::npos)
			continue;

		std::string	key = Utils::strToLower(Utils::trim(line.substr(0, colon), Http::SPACE_CHARS));
		std::string	val = Utils::trim(line.substr(colon + 1), Http::SPACE_CHARS);

		if (key == "status")
		{
			int	code = std::atoi(val.c_str());
			if (code >= 100 && code <= 599)
				resp.setStatus((Http::StatusCode)code);
		}
		else if (key == "content-type")
			contentType = val;
		else if (key != "content-length")
			resp.setHeader(key, val);
	}

	resp.setBody(body, contentType);
	return resp;
}
