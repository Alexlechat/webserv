#ifndef FileLogger_HPP
# define FileLogger_HPP

# include "logger/Logger.hpp"
# include "config/FileLoggerConfig.hpp"

# include <fstream>

class	FileLogger : public Logger
{
	public:
		static FileLogger& instance(void)
		{
			static FileLogger	inst(FileLoggerConfig(DEFAULT_ERROR_LOG_FILEPATH, DEFAULT_ERROR_LOG_MIN_LEVEL));
			return inst;
		}

		explicit FileLogger(const FileLoggerConfig& fileLoggerConfig, bool truncate = true);
		virtual ~FileLogger(void);

		virtual void	log(const std::string& msg, LogLevel level, const char* file, int line);

		bool			isOpen(void) const;

	private:
		std::ofstream	_file;
		
		FileLogger(const FileLogger&);
		FileLogger& operator=(const FileLogger&);
};

#endif  // FileLogger_HPP
