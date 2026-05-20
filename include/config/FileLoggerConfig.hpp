#ifndef FileLoggerConfig_HPP
# define FileLoggerConfig_HPP

# include "logger/Logger.hpp"
# include "logger/ConsoleLogger.hpp"

# include <string>

# define DEFAULT_ERROR_LOG_FILEPATH "logs/error.log"

struct FileLoggerConfig
{
	std::string			log_filepath;
	Logger::LogLevel	log_min_lvl;

	FileLoggerConfig(void) {}
	FileLoggerConfig(const FileLoggerConfig& src) : log_filepath(src.log_filepath), log_min_lvl(src.log_min_lvl) {}
	FileLoggerConfig(const char* filepath, const char* min_lvl) : log_filepath(filepath), log_min_lvl(Logger::stringToLevel(min_lvl)) {}
	FileLoggerConfig(const std::string& filepath, Logger::LogLevel min_lvl) : log_filepath(filepath), log_min_lvl(min_lvl) {}

	void	setErrorLogs(std::string filepath, std::string min_lvl)
	{ 
		log_filepath = filepath;
		log_min_lvl = Logger::stringToLevel(min_lvl);

		if (log_min_lvl == Logger::UNKNOWN)
		{
			LOG_WARNING(ConsoleLogger::instance(), "Invalid Logger minimal level, defaulting to ERROR");
			log_min_lvl = Logger::ERR;
		}
	}
};

#endif  // FileLoggerConfig_HPP#ifndef LogConfig_HPP
