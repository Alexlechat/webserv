#ifndef HttpResponseBuilder_HPP
# define HttpResponseBuilder_HPP

# include "http/Http.hpp"
# include "http/HttpRequest.hpp"
# include "http/HttpResponse.hpp"
# include "server/Server.hpp"
# include "config/ServerConfig.hpp"
# include "config/LocationConfig.hpp"

class	HttpResponseBuilder
{
	// ResponseBuilder.hpp — additions only
	struct MultipartPart
	{
		std::map<std::string, std::string> headers;
		std::string                        body;
	};

	public:
		HttpResponseBuilder(const HttpRequest& request, const Server& server);
		HttpResponse	build(void);

	private:
		const HttpRequest&		_request;
		const Server&			_server;
		const ServerConfig&		_config;
		const LocationConfig*	_location;

		const LocationConfig*	_matchLocation(void) const;
		std::string				_resolvePath(void) const;

		HttpResponse	_handleGet(void);
		HttpResponse	_handlePost(void);
		HttpResponse	_handleDelete(void);


		// replace _handleUpload declaration, add helpers
		HttpResponse               _handleUpload(void);
		HttpResponse               _handleMultipartUpload(const std::string& boundary);
		HttpResponse               _handleRawUpload(void);
		HttpResponse               _saveUploadedFile(const std::string& filename, const std::string& content);
		std::vector<MultipartPart> _parseMultipart(const std::string& boundary) const;
		std::string                _extractFilename(const std::string& disposition) const;
		std::string                _extFromContentType(const std::string& ct) const;

		HttpResponse	_serveFile(const std::string& fsPath);
		HttpResponse	_serveDirectory(const std::string& fsPath);
		HttpResponse	_autoIndex(const std::string& fsPath) const;
		HttpResponse	_executeCgi(const std::string& fsPath, const std::string& interpreter);
		HttpResponse	_parseCgiOutput(const std::string& output);
		HttpResponse	_buildError(Http::StatusCode code) const;

		bool			_isMethodAllowed(void) const;
		bool			_matchCgi(const std::string& path, std::string& outInterpreter) const;
		std::string		_getContentType(const std::string& path) const;
		std::string		_readFile(const std::string& path, bool& ok) const;

		HttpResponseBuilder(const HttpResponseBuilder&);
		HttpResponseBuilder&	operator=(const HttpResponseBuilder&);
};

HttpResponse	buildResponse(const HttpRequest& request, const Server& server);

#endif  // HttpResponseBuilder_HPP
