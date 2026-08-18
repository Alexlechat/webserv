#ifndef ConsoleLogger_HPP
# define ConsoleLogger_HPP

# include "logger/Logger.hpp"

class	ConsoleLogger : public Logger
{
	public:
		static ConsoleLogger& instance(void)
		{
			static ConsoleLogger inst(true);
			return inst;
		}

		explicit ConsoleLogger(bool useColor = true);
		virtual ~ConsoleLogger(void) {}

		virtual void	log(const std::string& msg) const;
		virtual void	logLevel(const std::string& msg, LogLevel level) const;
		void			setColor(bool enable) { _useColor = enable; }

	private:
		bool	_useColor;
};

#endif
