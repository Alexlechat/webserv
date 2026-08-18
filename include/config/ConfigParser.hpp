#ifndef ConfigParser_HPP
# define ConfigParser_HPP

# include <map>
# include <string>
# include <vector>
# include <exception>

# include "config/HttpConfig.hpp"
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
		HttpConfig			_http_ctx;

		typedef void		(*HttpHandler)(ConfigParser&, Config&);
		static std::map<std::string, HttpHandler>		_makeHttpHandlers(void);

		typedef void		(*ServerHandler)(ConfigParser&, Config&);
		static std::map<std::string, ServerHandler>		_makeServerHandlers(void);

		typedef void		(*LocationHandler)(ConfigParser&, Config&);
		static std::map<std::string, LocationHandler>	_makeLocationHandlers(void);

		static void			_handleServer(ConfigParser& p, Config& cfg);
		static void			_handleLocation(ConfigParser& p, Config& cfg);

		static void			_handleErrorLog(ConfigParser& p, Config& cfg);
		static void			_handleAccessLog(ConfigParser& p, Config& cfg);

		static void			_handleRoot(ConfigParser& p, Config& cfg);
		static void			_handleIndex(ConfigParser& p, Config& cfg);
		static void			_handleClientMaxBodySize(ConfigParser& p, Config& cfg);
		static void			_handleErrorPages(ConfigParser& p, Config& cfg);
		static void			_handleCgiExtensions(ConfigParser& p, Config& cfg);

		static void			_handleListen(ConfigParser& p, Config& cfg);
		static void			_handleServerName(ConfigParser& p, Config& cfg);

		static void			_handlePath(ConfigParser& p, Config& cfg);
		static void			_handleAutoindex(ConfigParser& p, Config& cfg);
		static void			_handleUploadStore(ConfigParser& p, Config& cfg);
		static void			_handleUploadReturn(ConfigParser& p, Config& cfg);
		static void			_handleMethods(ConfigParser& p, Config& cfg);
		static void			_handleRedirects(ConfigParser& p, Config& cfg);

		static void			_inheritShared(Config& child, const Config& parent);
		void				_resolveInheritance(void);

		void				_parse(void);
		HttpConfig			_parseHttp(void);
		ServerConfig		_parseServer(void);
		LocationConfig		_parseLocation(const std::string& path);

		std::vector<std::string>	_tokens;
		size_t						_pos;

		void				_tokenize(const std::string& fileContent);
		void				_expect(const std::string& token);
		bool				_atEnd(void) const;
		const std::string&	_current(void) const;
		const std::string&	_consume(void);
};

#endif
