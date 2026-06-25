#ifndef ConfigParser_HPP
# define ConfigParser_HPP

# include <string>
# include <vector>
# include <exception>

# include "config/ServerConfig.hpp"
# include "config/LocationConfig.hpp"

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
		std::vector<ServerConfig>	_serversConfig;
		FileLoggerConfig			_globalErrorFileLoggerConfig;
		FileLoggerConfig			_globalAccessFileLoggerConfig;

		typedef void		(*LocationHandler)(ConfigParser&, LocationConfig&);

		static void			_handleRoot(ConfigParser& p, LocationConfig& cfg);
		static void			_handleIndex(ConfigParser& p, LocationConfig& cfg);
		static void			_handleMethods(ConfigParser& p, LocationConfig& cfg);
		static void			_handleAutoindex(ConfigParser& p, LocationConfig& cfg);
		static void			_handleUploadStore(ConfigParser& p, LocationConfig& cfg);
		static void			_handleUploadReturnUrl(ConfigParser& p, LocationConfig& cfg);
		static void			_handleRedirect(ConfigParser& p, LocationConfig& cfg);
		static void			_handleCgiExtension(ConfigParser& p, LocationConfig& cfg);

		static std::map<std::string, LocationHandler>	_makeLocationHandlers(void);

		typedef void		(*ServerHandler)(ConfigParser&, ServerConfig&);

		static void			_handleListen(ConfigParser& p, ServerConfig& cfg);
		static void			_handleServerName(ConfigParser& p, ServerConfig& cfg);
		static void			_handleClientMaxBodySize(ConfigParser& p, ServerConfig& cfg);
		static void			_handleErrorPage(ConfigParser& p, ServerConfig& cfg);
		static void			_handleLocation(ConfigParser& p, ServerConfig& cfg);
		static void			_handleErrorLog(ConfigParser& p, ServerConfig& cfg);
		static void			_handleAccessLog(ConfigParser& p, ServerConfig& cfg);

		static std::map<std::string, ServerHandler>		_makeServerHandlers(void);

		void				_parse(void);
		ServerConfig		_parseServer(void);
		LocationConfig		_parseLocation(const std::string& path);
		void				_tokenize(const std::string& fileContent);


		std::vector<std::string>	_tokens;
		size_t						_pos;

		void				_expect(const std::string& token);
		bool				_atEnd(void) const;
		const std::string&	_current(void) const;
		const std::string&	_consume(void);
};

#endif  // ConfigParser_HPP
