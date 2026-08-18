#include "logger/MultiLogger.hpp"

void	MultiLogger::addLogger(const Logger& logger) { _loggers.push_back(&logger); }

void	MultiLogger::logLevel(const std::string& msg, LogLevel level) const
{
	for (size_t i = 0; i < _loggers.size(); i++)
	{
		_loggers[i]->logLevel(msg, level);
	}
}

void	MultiLogger::log(const std::string& msg) const
{
	for (size_t i = 0; i < _loggers.size(); i++)
	{
		_loggers[i]->log(msg);
	}
}
