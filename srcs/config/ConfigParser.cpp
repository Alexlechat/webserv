#include "config/ConfigParser.hpp"
#include "config/ServerConfig.hpp"
#include "config/LocationConfig.hpp"
#include "config/FileLoggerConfig.hpp"

#include <cctype>
#include <cstdlib>
#include <fstream>
#include <sstream>

static bool	_isSingleCharToken(char c);

ConfigParser::ConfigParser(const std::string& configFilePath) : _pos(0)
{
	std::string	fileContent = _getFileContent(configFilePath);

	_globalErrorFileLoggerConfig = FileLoggerConfig(FileLoggerConfig::DEFAULT_ERROR);
	_globalAccessFileLoggerConfig = FileLoggerConfig(FileLoggerConfig::DEFAULT_ACCESS);
	_tokenize(fileContent);
	_parse();
}

const std::vector<ServerConfig>&	ConfigParser::getServerConfigs(void) const { return _serversConfig; }

std::string	ConfigParser::_getFileContent(const std::string& configFilePath)
{
	std::ifstream	file(configFilePath.c_str());

	if (!file)
		throw ParseException("Failed to open Config File: " + configFilePath);

	std::string			fileContent;
	std::stringstream	ss;

	ss << file.rdbuf();
	fileContent = ss.str();

	return fileContent;
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

void	ConfigParser::_parse(void)
{
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

	while (_current() != "}")
	{
		std::string	key = _consume();

		if (key == "listen")
		{
			serverConfig.setPort(_consume());
			_expect(";");
		}
		else if (key == "server_name")
		{
			serverConfig.setServerName(_consume());
			_expect(";");
		}
		else if (key == "client_max_body_size")
		{
			serverConfig.setClientMaxBodySize(_consume());
			_expect(";");
		}
		else if (key == "error_page")
		{
			const std::string&	code = _consume();
			const std::string&	page = _consume();
			serverConfig.setErrorPages(code, page);
			_expect(";");
		}
		else if (key == "location")
		{
			serverConfig.setLocations(_parseLocation(_consume()));
		}
		else if (key == "error_log")
		{
			const std::string& filepath = _consume();
			const std::string& min_log_level = _consume();
			serverConfig.errorLogConfig.setErrorFileLogger(filepath, min_log_level);
			_expect(";");
		}
		else if (key == "access_log")
		{
			serverConfig.accessLogConfig.setAccessFileLogger(_consume());
			_expect(";");
		}
		else throw ParseException("Unexpected server directive: " + key);

	}

	_expect("}");
	return serverConfig;
}

LocationConfig	ConfigParser::_parseLocation(const std::string& path)
{
	_expect("{");

	LocationConfig	locationConfig;
	locationConfig.setPath(path);

	while (_current() != "}")
	{
		std::string	key = _consume();

		if (key == "root")
		{
			locationConfig.setRoot(_consume());
			_expect(";");
		}
		else if (key == "index")
		{
			locationConfig.setIndex(_consume());
			_expect(";");
		}
		else if (key == "methods")
		{
			while (_current() != ";")
				locationConfig.setMethods(_consume());
			_expect(";");
		}
		else if (key == "autoindex")
		{
			locationConfig.setAutoindex(_consume());
			_expect(";");
		}
		else if (key == "upload_store")
		{
			locationConfig.setUploadStore(_consume());
			_expect(";");
		}
		else if (key == "redirect")
		{
			const std::string&	code = _consume();
			const std::string&	url = _consume();
			locationConfig.setRedirect(code, url);
			_expect(";");
		}
		else if (key == "cgi_extension")
		{
			const std::string&	extension = _consume();
			const std::string&	interpreter = _consume();
			locationConfig.setCgiExtensions(extension, interpreter);
			_expect(";");
		}
		else throw ParseException("Unexpected location directive: " + key);
	}

	_expect("}");
	return locationConfig;
}

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
