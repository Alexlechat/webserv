#ifndef MultiLogger_HPP
# define MultiLogger_HPP

# include "logger/Logger.hpp"

# include <vector>

class	MultiLogger : public Logger
{
	public:
		MultiLogger(void) : Logger() {}
		virtual ~MultiLogger(void) {}

		void			addLogger(const Logger& logger);
		virtual void	log(const std::string& msg) const;
		virtual void	logLevel(const std::string& msg, LogLevel level) const;

	private:
		std::vector<const Logger*>	_loggers;
};

#endif
