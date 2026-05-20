#include "logger/ConsoleLogger.hpp"

#include <iostream>
 
static const std::string	RESET   = "\033[0m";
 
ConsoleLogger::ConsoleLogger(bool useColor) : Logger(), _useColor(useColor) {}
 
void	ConsoleLogger::log(const std::string& msg, LogLevel level, const char* file, int line)
{
	if (!_shouldLog(level))
		return ;
 
	std::ostream&	out = (level >= WARN) ? std::cerr : std::cout;
	std::string		formatted = _formatMessage(msg, level, file, line);
 
	if (_useColor)
		out << _levelToColor(level) << formatted << RESET << "\n";
	else
		out << formatted << "\n";
}
