#include "logger/Logger.hpp"

#include <ctime>
#include <iomanip>
#include <sstream>

static const std::string	RESET   = "\033[0m";
static const std::string	GREY    = "\033[90m";
static const std::string	CYAN    = "\033[36m";
static const std::string	YELLOW  = "\033[33m";
static const std::string	RED     = "\033[31m";
static const std::string	MAGENTA = "\033[35;1m";

std::string	Logger::_levelToString(LogLevel level) const
{
	switch (level)
	{
		case DEBUG:	   return "DEBUG";
		case INFO:     return "INFO";
		case WARNING:  return "WARNING";
		case ERROR:    return "ERROR";
		case CRITICAL: return "CRITICAL";
		default:	   return "UNKNOWN";
	}
}

std::string		Logger::_levelToColor(LogLevel level) const
{
	switch (level)
	{
		case DEBUG:	   return GREY;
		case INFO:     return CYAN;
		case WARNING:  return YELLOW;
		case ERROR:    return RED;
		case CRITICAL: return MAGENTA;
		default:	   return RESET;
	}
}

std::string		Logger::_formatMessage(const std::string& msg, LogLevel level, const char* file, int line) const
{
	std::time_t	t = std::time(NULL);
	std::tm*	tm = std::localtime(&t);
 
	std::ostringstream oss;
	oss << "["
		<< std::setfill('0')
		<< (1900 + tm->tm_year) << "-"
		<< std::setw(2) << (1 + tm->tm_mon) << "-"
		<< std::setw(2) << tm->tm_mday << " "
		<< std::setw(2) << tm->tm_hour << ":"
		<< std::setw(2) << tm->tm_min  << ":"
		<< std::setw(2) << tm->tm_sec
		<< "] "
		<< "[" << _levelToString(level) << "] "
		<< msg
		<< "  (" << file << ":" << line << ")";
 
	return oss.str();
}

bool				Logger::_shouldLog(LogLevel level) const { return level >= _minLevel; }
void				Logger::setMinLevel(LogLevel level) { _minLevel = level; }
Logger::LogLevel	Logger::getMinLevel(void) const { return _minLevel; }
