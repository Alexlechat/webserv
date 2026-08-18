#ifndef FileLogger_HPP
# define FileLogger_HPP

# include "logger/Logger.hpp"
# include "config/FileLoggerConfig.hpp"

# include <fstream>

class	FileLogger : public Logger
{
	public:
		explicit FileLogger(const FileLoggerConfig& fileLoggerConfig, bool truncate = false);
		virtual ~FileLogger(void);

		virtual void	log(const std::string& msg) const;
		virtual void	logLevel(const std::string& msg, LogLevel level) const;

		bool			isOpen(void) const;

	private:
		mutable std::ofstream	_file;

		FileLogger(const FileLogger&);
		FileLogger& operator=(const FileLogger&);
};

#endif
