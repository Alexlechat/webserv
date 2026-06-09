#ifndef FileLoggerConfig_HPP
# define FileLoggerConfig_HPP

# include "logger/Logger.hpp"
# include "logger/ConsoleLogger.hpp"

# include <string>

# define DEFAULT_ERROR_LOG_MIN_LEVEL "error"
# define DEFAULT_ERROR_LOG_FILEPATH "logs/error.log"

# define DEFAULT_ACCESS_LOG_FILEPATH "logs/access.log"

struct FileLoggerConfig
{
	enum DefaultFileConfig
	{
		DEFAULT_ERROR  = 0,
		DEFAULT_ACCESS = 1,
	};

	std::string			log_filepath;
	Logger::LogLevel	log_min_lvl;

	FileLoggerConfig(void) {}
	FileLoggerConfig(DefaultFileConfig def)
	{
		switch (def) 
		{
			case DEFAULT_ERROR:
				log_filepath = DEFAULT_ERROR_LOG_FILEPATH;
				log_min_lvl = Logger::stringToLevel(DEFAULT_ERROR_LOG_MIN_LEVEL);
				break ;
			case DEFAULT_ACCESS:
				log_filepath = DEFAULT_ACCESS_LOG_FILEPATH;
				log_min_lvl = Logger::DEBUG;
				break ;
			default:
				break;
		}
	}
	FileLoggerConfig(const FileLoggerConfig& src) : log_filepath(src.log_filepath), log_min_lvl(src.log_min_lvl) {}
	FileLoggerConfig(const char* filepath, const char* min_lvl) : log_filepath(filepath), log_min_lvl(Logger::stringToLevel(min_lvl)) {}
	FileLoggerConfig(const std::string& filepath, Logger::LogLevel min_lvl) : log_filepath(filepath), log_min_lvl(min_lvl) {}

	void	setErrorFileLogger(std::string filepath, std::string min_lvl)
	{ 
		log_filepath = filepath;
		log_min_lvl = Logger::stringToLevel(min_lvl);

		if (log_min_lvl == Logger::UNKNOWN)
		{
			LOG_WARNING(ConsoleLogger::instance(), "Invalid FileLogger minimal level, defaulting to ERROR");
			log_min_lvl = Logger::ERR;
		}
	}

	void	setAccessFileLogger(std::string filepath) { log_filepath = filepath; }
};

#endif  // FileLoggerConfig_HPP#ifndef LogConfig_HPP
