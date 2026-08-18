#include "config/ConfigParser.hpp"
#include "config/HttpConfig.hpp"
#include "config/ServerConfig.hpp"
#include "config/LocationConfig.hpp"
#include "config/FileLoggerConfig.hpp"
#include "logger/Logger.hpp"
#include "utils/Utils.hpp"

static bool	_isSingleCharToken(char c);

ConfigParser::ConfigParser(const std::string& configFilePath) : _pos(0)
{
	std::string	fileContent = Utils::readFile(configFilePath);
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
		if (std::isspace((unsigned char)fileContent[i])) { i++; continue ; }

		if (fileContent[i] == '#')
		{
			while (i < fileContent.size() && fileContent[i] != '\n')
				i++;
			continue ;
		}

		if (_isSingleCharToken(fileContent[i]))
		{
			_tokens.push_back(std::string(1, fileContent[i++]));
			continue ;
		}

		size_t	start = i;
		while (i < fileContent.size() && !std::isspace((unsigned char)fileContent[i]) && !_isSingleCharToken(fileContent[i]))
			++i;

		_tokens.push_back(fileContent.substr(start, i - start));
	}
}

void	ConfigParser::_handleRoot(ConfigParser& p, Config& cfg)
{
	cfg.setRoot(p._consume());
	p._expect(";");
}

void	ConfigParser::_handleIndex(ConfigParser& p, Config& cfg)
{
	cfg.setIndex(p._consume());
	p._expect(";");
}

void	ConfigParser::_handleClientMaxBodySize(ConfigParser& p, Config& cfg)
{
	cfg.setClientMaxBodySize(p._consume());
	p._expect(";");
}

void	ConfigParser::_handleErrorPages(ConfigParser& p, Config& cfg)
{
	const std::string&	code = p._consume();
	const std::string&	page = p._consume();

	cfg.setErrorPages(code, page);
	p._expect(";");
}

void	ConfigParser::_handleCgiExtensions(ConfigParser& p, Config& cfg)
{
	const std::string&	ext = p._consume();
	const std::string&	interp = p._consume();

	cfg.setCgiExtensions(ext, interp);
	p._expect(";");
}

void	ConfigParser::_handleErrorLog(ConfigParser& p, Config& cfg)
{
	const std::string&	filepath = p._consume();
	std::string			min_log_level = "error";

	if (p._current() != ";")
		min_log_level = p._consume();

	cfg.error_log_config.value.setErrorFileLogger(filepath, min_log_level);
	cfg.error_log_config.specified = true;
	p._expect(";");
}

void	ConfigParser::_handleAccessLog(ConfigParser& p, Config& cfg)
{
	cfg.access_log_config.value.setAccessFileLogger(p._consume());
	cfg.access_log_config.specified = true;
	p._expect(";");
}

void	ConfigParser::_handleListen(ConfigParser& p, Config& cfg)
{
	static_cast<ServerConfig&>(cfg).setListen(p._consume());
	p._expect(";");
}

void	ConfigParser::_handleServerName(ConfigParser& p, Config& cfg)
{
	static_cast<ServerConfig&>(cfg).setServerName(p._consume());
	p._expect(";");
}

void	ConfigParser::_handlePath(ConfigParser& p, Config& cfg)
{
	static_cast<LocationConfig&>(cfg).setPath(p._consume());
	p._expect(";");
}

void	ConfigParser::_handleAutoindex(ConfigParser& p, Config& cfg)
{
	static_cast<LocationConfig&>(cfg).setAutoindex(p._consume());
	p._expect(";");
}

void	ConfigParser::_handleUploadStore(ConfigParser& p, Config& cfg)
{
	static_cast<LocationConfig&>(cfg).setUploadStore(p._consume());
	p._expect(";");
}

void	ConfigParser::_handleUploadReturn(ConfigParser& p, Config& cfg)
{
	static_cast<LocationConfig&>(cfg).setUploadReturn(p._consume());
	p._expect(";");
}

void	ConfigParser::_handleMethods(ConfigParser& p, Config& cfg)
{
	LocationConfig&	loc = static_cast<LocationConfig&>(cfg);

	while (p._current() != ";")
		loc.setMethods(p._consume());
	p._expect(";");
}

void	ConfigParser::_handleRedirects(ConfigParser& p, Config& cfg)
{
	LocationConfig&		loc = static_cast<LocationConfig&>(cfg);
	const std::string&	code = p._consume();
	const std::string&	url = p._consume();

	loc.setRedirects(code, url);
	p._expect(";");
}

void	ConfigParser::_inheritShared(Config& child, const Config& parent)
{
	inherit(child.error_log_config, parent.error_log_config);
	inherit(child.access_log_config, parent.access_log_config);
	inherit(child.root, parent.root);
	inherit(child.index, parent.index);
	inherit(child.client_max_body_size, parent.client_max_body_size);
	inheritMap(child.error_pages, parent.error_pages);
	inheritMap(child.cgi_extensions, parent.cgi_extensions);
}

void	ConfigParser::_handleServer(ConfigParser& p, Config& cfg)
{
	HttpConfig&		httpCfg = static_cast<HttpConfig&>(cfg);
	ServerConfig	serverConfig = p._parseServer();

	if (!serverConfig.port.specified)
		throw ParseException("server{} block is missing the mandatory 'listen' directive");

	httpCfg.servers.push_back(serverConfig);
}

void	ConfigParser::_handleLocation(ConfigParser& p, Config& cfg)
{
	ServerConfig&	serverCfg = static_cast<ServerConfig&>(cfg);
	std::string		path = p._consume();
	LocationConfig	locationConfig = p._parseLocation(path);

	serverCfg.locations.push_back(locationConfig);
}

std::map<std::string, ConfigParser::HttpHandler>	ConfigParser::_makeHttpHandlers(void)
{
	std::map<std::string, HttpHandler>	m;

	m["root"]					= &ConfigParser::_handleRoot;
	m["index"]					= &ConfigParser::_handleIndex;
	m["client_max_body_size"]	= &ConfigParser::_handleClientMaxBodySize;
	m["error_page"]				= &ConfigParser::_handleErrorPages;
	m["cgi_extension"]			= &ConfigParser::_handleCgiExtensions;
	m["error_log"]				= &ConfigParser::_handleErrorLog;
	m["access_log"]				= &ConfigParser::_handleAccessLog;
	m["server"]					= &ConfigParser::_handleServer;

	return m;
}

std::map<std::string, ConfigParser::ServerHandler>	ConfigParser::_makeServerHandlers(void)
{
	std::map<std::string, ServerHandler>	m;

	m["root"]					= &ConfigParser::_handleRoot;
	m["index"]					= &ConfigParser::_handleIndex;
	m["client_max_body_size"]	= &ConfigParser::_handleClientMaxBodySize;
	m["error_page"]				= &ConfigParser::_handleErrorPages;
	m["cgi_extension"]			= &ConfigParser::_handleCgiExtensions;
	m["error_log"]				= &ConfigParser::_handleErrorLog;
	m["access_log"]				= &ConfigParser::_handleAccessLog;
	m["listen"]					= &ConfigParser::_handleListen;
	m["server_name"]			= &ConfigParser::_handleServerName;
	m["location"]				= &ConfigParser::_handleLocation;

	return m;
}

std::map<std::string, ConfigParser::LocationHandler>	ConfigParser::_makeLocationHandlers(void)
{
	std::map<std::string, LocationHandler>	m;

	m["root"]					= &ConfigParser::_handleRoot;
	m["index"]					= &ConfigParser::_handleIndex;
	m["client_max_body_size"]	= &ConfigParser::_handleClientMaxBodySize;
	m["error_page"]				= &ConfigParser::_handleErrorPages;
	m["cgi_extension"]			= &ConfigParser::_handleCgiExtensions;
	m["error_log"]				= &ConfigParser::_handleErrorLog;
	m["access_log"]				= &ConfigParser::_handleAccessLog;
	m["path"]					= &ConfigParser::_handlePath;
	m["autoindex"]				= &ConfigParser::_handleAutoindex;
	m["upload_store"]			= &ConfigParser::_handleUploadStore;
	m["upload_return"]			= &ConfigParser::_handleUploadReturn;
	m["methods"]				= &ConfigParser::_handleMethods;
	m["redirect"]				= &ConfigParser::_handleRedirects;

	return m;
}

void	ConfigParser::_parse(void)
{
	bool	httpSeen = false;

	while (!_atEnd())
	{
		if (_current() != "http")
			throw ParseException("Expected 'http' block, got: " + _current());

		if (httpSeen)
			throw ParseException("Duplicate 'http' block: a config file may only have one");

		_consume();
		_http_ctx = _parseHttp();
		httpSeen = true;
	}

	if (!httpSeen)
		throw ParseException("Missing 'http' block in config file");

	if (_http_ctx.servers.empty())
		throw ParseException("No server block in http block");

	_resolveInheritance();
}

void	ConfigParser::_resolveInheritance(void)
{
	for (size_t i = 0; i < _http_ctx.servers.size(); ++i)
	{
		ServerConfig&	server = _http_ctx.servers[i];

		_inheritShared(server, _http_ctx);

		for (size_t j = 0; j < server.locations.size(); ++j)
			_inheritShared(server.locations[j], server);
	}
}

HttpConfig	ConfigParser::_parseHttp(void)
{
	_expect("{");

	HttpConfig	httpConfig;

	httpConfig.error_log_config.set(FileLoggerConfig(FileLoggerConfig::DEFAULT_ERROR));
	httpConfig.access_log_config.set(FileLoggerConfig(FileLoggerConfig::DEFAULT_ACCESS));

	static std::map<std::string, HttpHandler>	handlers = _makeHttpHandlers();

	while (_current() != "}")
	{
		const std::string	key = _consume();

		std::map<std::string, HttpHandler>::iterator	it = handlers.find(key);
		if (it == handlers.end())
			throw ParseException("Unexpected http directive: " + key);

		it->second(*this, httpConfig);
	}

	_expect("}");
	return httpConfig;
}

ServerConfig	ConfigParser::_parseServer(void)
{
	_expect("{");

	ServerConfig	serverConfig;

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

	const std::set<std::string>::iterator	it = locationConfig.methods.find("POST");
	if (it != locationConfig.methods.end() && locationConfig.upload_store->empty())
			LOG_WARNING(ConsoleLogger::instance(),
			"POST method found in location directive without upload_store set, non-CGI POST method will not be allowed for this location");

	_expect("}");
	return locationConfig;
}

const std::vector<ServerConfig>&	ConfigParser::getServerConfigs(void) const { return _http_ctx.servers; }

void	ConfigParser::_expect(const std::string& token)
{
	std::string buffer = _current();
	if (_consume() != token)
		throw ParseException("Unexpected token found: '" + buffer + "', expected: '" + token + "'");
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
