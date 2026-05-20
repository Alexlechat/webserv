#ifndef ConsoleLogger_HPP
# define ConsoleLogger_HPP

# include "logger/Logger.hpp"

class	ConsoleLogger : public Logger
{
	public:
		explicit ConsoleLogger(bool useColor = true);
		virtual ~ConsoleLogger(void) {}

		virtual void	log(const std::string& msg, LogLevel level, const char* file, int line);
		void			setColor(bool enable) { _useColor = enable; }

	private:
		bool	_useColor;
};

#endif  // ConsoleLogger_HPP
