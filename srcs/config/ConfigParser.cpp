#include "ConfigParser.hpp"

#include <cctype>
#include <fstream>
#include <sstream>
#include <iostream>

static bool	isSingleCharToken(char c);

ConfigParser::ConfigParser(const std::string& configFilePath) : _pos(0)
{
	std::string	fileContent = _getFileContent(configFilePath);

	(void)_pos;
	_tokenize(fileContent);
}

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

		if (isSingleCharToken(fileContent[i]))
		{
			_tokens.push_back(std::string(1, fileContent[i++]));
			continue ;
		}

		size_t	start = i;
		while (i < fileContent.size() && !std::isspace(fileContent[i]) && !isSingleCharToken(fileContent[i]))
			++i;

		_tokens.push_back(fileContent.substr(start, i - start));
	}
}

static bool	isSingleCharToken(char c)
{
	switch (c)
		case '#':
		case '{':
		case '}':
		case ';':
			return true;

	return false;
}
