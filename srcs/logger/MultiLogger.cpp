#include "logger/MultiLogger.hpp"

void	MultiLogger::addLogger(Logger* logger) { _loggers.push_back(logger); }

void	MultiLogger::log(const std::string& msg, LogLevel level, const char* file, int line)
{
	for (size_t i = 0; i < _loggers.size(); i++)
	{
		_loggers[i]->log(msg, level, file, line);
	}
}
