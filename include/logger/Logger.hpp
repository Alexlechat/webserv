#ifndef Logger_HPP
# define Logger_HPP

# include <string>

class	Logger
{
	public:
		enum LogLevel
		{
			DEBUG = 0,
			INFO  = 1,
			WARN  = 2,
			ERR   = 3,
			CRIT  = 4,
			UNKNOWN = 10
		};

		virtual ~Logger(void) {}

		virtual void		log(const std::string& msg) const = 0;
		virtual void		logLevel(const std::string& msg, LogLevel level) const = 0;

		void				setMinLevel(LogLevel level);
		LogLevel			getMinLevel(void) const;
		static LogLevel		stringToLevel(std::string s);

	protected:
		Logger(void) : _minLevel(DEBUG) {}
		Logger(Logger::LogLevel minLevel) : _minLevel(minLevel) {}

		bool			_shouldLog(LogLevel level) const;
		std::string		_levelToColor(LogLevel level) const;
		std::string		_levelToString(LogLevel level) const;

		std::string		_formatTime(void) const;
		std::string		_formatMessage(const std::string& msg) const;
		std::string		_formatLevelMessage(const std::string& msg, LogLevel level) const;

	private:
		LogLevel	_minLevel;
};

# define LOG(logger, message) (logger).log(message)
# define LOG_LVL(logger, level, message) (logger).logLevel(message, level)

# define LOG_DEBUG(logger, message)		LOG_LVL(logger, Logger::DEBUG,    message)
# define LOG_INFO(logger, message)		LOG_LVL(logger, Logger::INFO,     message)
# define LOG_WARNING(logger, message)	LOG_LVL(logger, Logger::WARN,  message)
# define LOG_ERROR(logger, message)		LOG_LVL(logger, Logger::ERR,    message)
# define LOG_CRITICAL(logger, message)	LOG_LVL(logger, Logger::CRIT, message)

#endif
