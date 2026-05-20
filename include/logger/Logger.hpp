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

		virtual void	log(const std::string& msg, LogLevel level, const char* file, int line) = 0;

		void				setMinLevel(LogLevel level);
		LogLevel			getMinLevel(void) const;
		static LogLevel		stringToLevel(std::string s);

	protected:
		Logger(void) : _minLevel(DEBUG) {}

		bool			_shouldLog(LogLevel level) const;
		std::string		_levelToColor(LogLevel level) const;
		std::string		_levelToString(LogLevel level) const;
		std::string		_formatMessage(const std::string& msg, LogLevel level, const char* file, int line) const;

	private:
		LogLevel	_minLevel;
};

# define LOG(logger, level, message) (logger).log(message, level, __FILE__, __LINE__)

# define LOG_DEBUG(logger, message)		LOG(logger, Logger::DEBUG,    message)
# define LOG_INFO(logger, message)		LOG(logger, Logger::INFO,     message)
# define LOG_WARNING(logger, message)	LOG(logger, Logger::WARN,  message)
# define LOG_ERROR(logger, message)		LOG(logger, Logger::ERR,    message)
# define LOG_CRITICAL(logger, message)	LOG(logger, Logger::CRIT, message)

# define LOG_FD(logger, level, socket, msg) \
    LOG((logger), (level), Utils::fdInBrackets((socket)->getSocketFd()) + " " + (msg))

# define LOG_FD_DEBUG(logger, socket, msg)    LOG_FD(logger, Logger::DEBUG,    socket, msg)
# define LOG_FD_INFO(logger, socket, msg)     LOG_FD(logger, Logger::INFO,     socket, msg)
# define LOG_FD_WARNING(logger, socket, msg)  LOG_FD(logger, Logger::WARN,  socket, msg)
# define LOG_FD_ERROR(logger, socket, msg)    LOG_FD(logger, Logger::ERR,    socket, msg)
# define LOG_FD_CRITICAL(logger, socket, msg) LOG_FD(logger, Logger::CRIT, socket, msg)

#endif  // Logger_HPP
