#include "logger/FileLogger.hpp"
#include "logger/Logger.hpp"
#include <stdexcept>

FileLogger::FileLogger(const FileLoggerConfig& fileLoggerConfig, bool truncate)
{
	std::ios::openmode	mode = std::ios::out;
	mode |= truncate ? std::ios::trunc : std::ios::app;

	if (fileLoggerConfig.log_min_lvl == UNKNOWN)
		throw std::runtime_error("FileLogger: Invalid minimal log level");

	_file.open(fileLoggerConfig.log_filepath.c_str(), mode);
	if (!_file.is_open())
		throw std::runtime_error("FileLogger: cannot open file: " + fileLoggerConfig.log_filepath);
}

FileLogger::~FileLogger(void)
{
	if (_file.is_open())
		_file.close();
}

void	FileLogger::log(const std::string& msg, Logger::LogLevel level, const char* file, int line)
{
	if (!_shouldLog(level) || !_file.is_open())
		return ;

	_file << _formatMessage(msg, level, file, line) << "\n";
	_file.flush();
}

bool	FileLogger::isOpen(void) const { return _file.is_open(); }
