#ifndef HttpResponseBuilder_HPP
# define HttpResponseBuilder_HPP

# include "http/Http.hpp"
# include "http/HttpRequest.hpp"
# include "http/HttpResponse.hpp"
# include "config/ServerConfig.hpp"
# include "config/LocationConfig.hpp"

class	Cgi;

class	HttpResponseBuilder
{
	public:
		// Result of build(): either a ready-to-send HttpResponse, or a
		// CGI that has been start()-ed (fork/pipe already done) and now
		// needs to be driven to completion by EventLoop's poll() loop.
		// When isCgi is true, ownership of `cgi` passes to the caller.
		struct BuildResult
		{
			bool			isCgi;
			HttpResponse	response;
			Cgi*			cgi;

			BuildResult(void) : isCgi(false), cgi(NULL) {}
		};

		HttpResponseBuilder(const HttpRequest& request, const ServerConfig& config);
		BuildResult			build(void);
		static HttpResponse	buildDefaultErrorPageBody(HttpResponse& resp, const Http::StatusCode code);

	private:
		const HttpRequest&		_request;
		const ServerConfig&		_config;
		const LocationConfig*	_location;

		const LocationConfig*	_matchLocation(void) const;
		const LocationConfig*	_matchLocationForPath(const std::string& path) const;
		bool					_resolvePath(std::string& outPath) const;
		std::string				_effectiveRoot(void) const;

		BuildResult		_handleGet(void);
		BuildResult		_handlePost(void);
		HttpResponse	_handleDelete(void);

		HttpResponse	_serveFile(const std::string& fsPath);
		BuildResult		_serveDirectory(const std::string& fsPath);
		HttpResponse	_autoIndex(const std::string& fsPath) const;
		HttpResponse	_handleUpload(void);
		BuildResult		_startCgi(const std::string& fsPath, const std::string& interpreter);
		HttpResponse	_buildError(Http::StatusCode code) const;

		bool			_isMethodAllowed(void) const;
		bool			_matchCgi(const std::string& path, std::string& outInterpreter) const;
		bool			_parseMultipartUpload(std::string& outFilename, std::string& outContent) const;
		std::string		_getContentType(const std::string& path) const;
		std::string		_readFile(const std::string& path, bool& ok) const;

		HttpResponseBuilder(const HttpResponseBuilder&);
		HttpResponseBuilder&	operator=(const HttpResponseBuilder&);
};

HttpResponseBuilder::BuildResult	buildResponse(const HttpRequest& request, const ServerConfig& config);

#endif  // HttpResponseBuilder_HPP
