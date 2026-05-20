#ifndef MultiLogger_HPP
# define MultiLogger_HPP

# include "logger/Logger.hpp"

# include <vector>

class	MultiLogger : public Logger
{
	public:
		MultiLogger(void) : Logger() {}
		virtual ~MultiLogger(void) {}

		void			addLogger(Logger* logger);
		virtual void	log(const std::string& msg, LogLevel level, const char* file, int line);

	private:
		std::vector<Logger*>	_loggers;
};

#endif  // MultiLogger_HPP
