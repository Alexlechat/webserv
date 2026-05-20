#ifndef ConfigParser_HPP
# define ConfigParser_HPP

# include <string>
# include <vector>
# include <exception>

# include "config/FileLoggerConfig.hpp"
# include "config/ServerConfig.hpp"
# include "config/LocationConfig.hpp"

# define DEFAULT_ERROR_LOG_MIN_LEVEL "error"
# define DEFAULT_ERROR_LOG_FILEPATH "logs/error.log"

class	ConfigParser
{
	public:
		ConfigParser(const std::string& configFilePath);

		const std::vector<ServerConfig>&	getServerConfigs(void) const;

		class	ParseException : public std::exception
		{
			public:
				ParseException(const std::string& msg) : _msg(msg) {}
				virtual const char* what(void) const throw() { return _msg.c_str(); }
				virtual ~ParseException() throw() {}

			private:
				std::string	_msg;
		};

	private:
		std::vector<std::string>	_tokens;
		size_t						_pos;

		std::vector<ServerConfig>	_serversConfig;
		FileLoggerConfig			_globalFileLoggerConfig;

		void				_parse(void);
		ServerConfig		_parseServer(void);
		LocationConfig		_parseLocation(const std::string& path);
		void				_tokenize(const std::string& fileContent);
		std::string			_getFileContent(const std::string& configFilePath);

		void				_expect(const std::string& token);
		bool				_atEnd(void) const;
		const std::string&	_current(void) const;
		const std::string&	_consume(void);

};

#endif  // ConfigParser_HPP
