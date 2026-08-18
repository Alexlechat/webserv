#include "logger/ConsoleLogger.hpp"

#include <iostream>

static const std::string	RESET   = "\033[0m";

ConsoleLogger::ConsoleLogger(bool useColor) : Logger(), _useColor(useColor) {}

void	ConsoleLogger::logLevel(const std::string& msg, LogLevel level) const
{
	if (!_shouldLog(level))
		return ;

	std::ostream&	out = (level >= WARN) ? std::cerr : std::cout;
	std::string		formatted = _formatLevelMessage(msg, level);

	if (_useColor)
		out << _levelToColor(level) << formatted << RESET << std::endl;
	else
		out << formatted << std::endl;
}

void	ConsoleLogger::log(const std::string& msg) const
{
	std::ostream&	out = std::cout;
	std::string		formatted = _formatMessage(msg);

	if (_useColor)
		out << _levelToColor(Logger::INFO) << formatted << RESET << std::endl;
	else
		out << formatted << std::endl;
}
