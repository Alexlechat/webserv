#include "config/ConfigParser.hpp"
#include "config/ServerConfig.hpp"
#include "config/LocationConfig.hpp"
#include "config/FileLoggerConfig.hpp"
#include "utils/Utils.hpp"

static bool	_isSingleCharToken(char c);

ConfigParser::ConfigParser(const std::string& configFilePath) : _pos(0)
{
	std::string	fileContent;

	fileContent = Utils::readFile(configFilePath);
	_tokenize(fileContent);

	LOG_INFO(ConsoleLogger::instance(), "Parsing config file at : " + configFilePath);
	_parse();
	LOG_INFO(ConsoleLogger::instance(), "Done parsing");
}

void	ConfigParser::_tokenize(const std::string& fileContent)
{
	size_t	i = 0;

	while (i < fileContent.size())
	{
		if (std::isspace(fileContent[i])) { i++; continue ; }

		if (fileContent[i] == '#')
			while (fileContent[i] != '\n' && i < fileContent.size())
				i++;

		if (_isSingleCharToken(fileContent[i]))
		{
			_tokens.push_back(std::string(1, fileContent[i++]));
			continue ;
		}

		size_t	start = i;
		while (i < fileContent.size() && !std::isspace(fileContent[i]) && !_isSingleCharToken(fileContent[i]))
			++i;

		_tokens.push_back(fileContent.substr(start, i - start));
	}
}

std::map<std::string, ConfigParser::LocationHandler>	ConfigParser::_makeLocationHandlers(void)
{
	std::map<std::string, ConfigParser::LocationHandler>	m_loc;

	m_loc["root"]			= &ConfigParser::_handleRoot;
	m_loc["index"]			= &ConfigParser::_handleIndex;
	m_loc["methods"]		= &ConfigParser::_handleMethods;
	m_loc["autoindex"]		= &ConfigParser::_handleAutoindex;
	m_loc["upload_store"]	= &ConfigParser::_handleUploadStore;
	m_loc["upload_return"]	= &ConfigParser::_handleUploadReturnUrl;
	m_loc["redirect"]		= &ConfigParser::_handleRedirect;
	m_loc["cgi_extension"]	= &ConfigParser::_handleCgiExtension;

	return m_loc;
}

void	ConfigParser::_handleRoot(ConfigParser& p, LocationConfig& cfg)
{
	cfg.setRoot(p._consume());
	p._expect(";");
}

void	ConfigParser::_handleIndex(ConfigParser& p, LocationConfig& cfg)
{
	cfg.setIndex(p._consume());
	p._expect(";");
}

void	ConfigParser::_handleMethods(ConfigParser& p, LocationConfig& cfg)
{
	while (p._current() != ";")
		cfg.setMethods(p._consume());
	p._expect(";");
}

void	ConfigParser::_handleAutoindex(ConfigParser& p, LocationConfig& cfg)
{
	cfg.setAutoindex(p._consume());
	p._expect(";");
}

void	ConfigParser::_handleUploadStore(ConfigParser& p, LocationConfig& cfg)
{
	cfg.setUploadStore(p._consume());
	p._expect(";");
}

void	ConfigParser::_handleUploadReturnUrl(ConfigParser& p, LocationConfig& cfg)
{
	cfg.setUploadReturnUrl(p._consume());
	p._expect(";");
}

void	ConfigParser::_handleRedirect(ConfigParser& p, LocationConfig& cfg)
{
	const std::string&	code = p._consume();
	const std::string&	url = p._consume();

	cfg.setRedirects(code, url);
	p._expect(";");
}

void	ConfigParser::_handleCgiExtension(ConfigParser& p, LocationConfig& cfg)
{
	const std::string&	ext = p._consume();
	const std::string&	interp = p._consume();

	cfg.setCgiExtensions(ext, interp);
	p._expect(";");
}

std::map<std::string, ConfigParser::ServerHandler>	ConfigParser::_makeServerHandlers(void)
{
	std::map<std::string, ConfigParser::ServerHandler>	m_server;

	m_server["listen"]					= &ConfigParser::_handleListen;
	m_server["server_name"]				= &ConfigParser::_handleServerName;
	m_server["client_max_body_size"]	= &ConfigParser::_handleClientMaxBodySize;
	m_server["error_page"]				= &ConfigParser::_handleErrorPage;
	m_server["location"]				= &ConfigParser::_handleLocation;
	m_server["error_log"]				= &ConfigParser::_handleErrorLog;
	m_server["access_log"]				= &ConfigParser::_handleAccessLog;

	return m_server;
}

void	ConfigParser::_handleListen(ConfigParser& p, ServerConfig& cfg)
{
	cfg.setPort(p._consume());
	p._expect(";");
}

void	ConfigParser::_handleServerName(ConfigParser& p, ServerConfig& cfg)
{
	cfg.setServerName(p._consume());
	p._expect(";");
}

void	ConfigParser::_handleClientMaxBodySize(ConfigParser& p, ServerConfig& cfg)
{
	cfg.setClientMaxBodySize(p._consume());
	p._expect(";");
}

void	ConfigParser::_handleErrorPage(ConfigParser& p, ServerConfig& cfg)
{
	const std::string&	code = p._consume();
	const std::string&	page = p._consume();

	cfg.setErrorPages(code, page);
	p._expect(";");
}

void	ConfigParser::_handleLocation(ConfigParser& p, ServerConfig& cfg)
{
	cfg.setLocations(p._parseLocation(p._consume()));
}

void	ConfigParser::_handleErrorLog(ConfigParser& p, ServerConfig& cfg)
{
	const std::string&	filepath = p._consume();
	const std::string&	min_log_level = p._consume();

	cfg.errorLogConfig.setErrorFileLogger(filepath, min_log_level);
	p._expect(";");
}

void	ConfigParser::_handleAccessLog(ConfigParser& p, ServerConfig& cfg)
{
	cfg.errorLogConfig.setAccessFileLogger(p._consume());
	p._expect(";");
}

void	ConfigParser::_parse(void)
{
	_globalErrorFileLoggerConfig = FileLoggerConfig(FileLoggerConfig::DEFAULT_ERROR);
	_globalAccessFileLoggerConfig = FileLoggerConfig(FileLoggerConfig::DEFAULT_ACCESS);

	while (!_atEnd())
	{
		if (_current() == "server")
		{
			_consume();
			_serversConfig.push_back(_parseServer());
		}
		else if (_current() == "error_log")
		{
			_consume();

			const std::string& filepath = _consume();
			const std::string& min_log_level = _consume();
			_globalErrorFileLoggerConfig.setErrorFileLogger(filepath, min_log_level);

			_expect(";");
		}
		else if (_current() == "access_log")
		{
			_consume();
			_globalAccessFileLoggerConfig.setAccessFileLogger(_consume());
			_expect(";");
		}
		else
			throw ParseException("Expected server directive, got: " +_current());
	}

	if (_serversConfig.size() == 0)
		throw ParseException("No server block in Config file");
}

ServerConfig	ConfigParser::_parseServer(void)
{
	_expect("{");

	ServerConfig	serverConfig;

	serverConfig.errorLogConfig = _globalErrorFileLoggerConfig;
	serverConfig.accessLogConfig = _globalAccessFileLoggerConfig;

	static std::map<std::string, ServerHandler>	handlers = _makeServerHandlers();

	while (_current() != "}")
	{
		const std::string	key = _consume();

		std::map<std::string, ServerHandler>::iterator	it = handlers.find(key);
		if (it == handlers.end())
			throw ParseException("Unexpected server directive: " + key);
		
		it->second(*this, serverConfig);
	}

	_expect("}");
	return serverConfig;
}

LocationConfig	ConfigParser::_parseLocation(const std::string& path)
{
	_expect("{");

	LocationConfig	locationConfig;
	locationConfig.setPath(path);

	static std::map<std::string, LocationHandler>	handlers = _makeLocationHandlers();

	while (_current() != "}")
	{
		const std::string	key = _consume();

		std::map<std::string, LocationHandler>::iterator	it = handlers.find(key);
		if (it == handlers.end())
			throw ParseException("Unexpected location directive: " + key);

		it->second(*this, locationConfig);
	}

	_expect("}");
	return locationConfig;
}

const std::vector<ServerConfig>&	ConfigParser::getServerConfigs(void) const { return _serversConfig; }

void	ConfigParser::_expect(const std::string& token)
{
	std::string buffer = _current();
	if (_consume() != token)
	{
		throw ParseException("Unexpected token found: '" + buffer + "', expected: " + "'" + token + "'");
	}
}

const std::string&	ConfigParser::_current(void) const
{
	if (_atEnd())
		throw ParseException("Unexpected end of input");

	return _tokens[_pos];
}

const std::string&	ConfigParser::_consume(void)
{
	const std::string&	token = _current();
	_pos++;

	return token;
}

bool	ConfigParser::_atEnd(void) const
{
	return _pos >= _tokens.size();
}

static bool	_isSingleCharToken(char c)
{
	switch (c)
		case '#':
		case '{':
		case '}':
		case ';':
			return true;

	return false;
}
