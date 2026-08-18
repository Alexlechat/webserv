#ifndef HttpResponse_HPP
# define HttpResponse_HPP

# include "http/Http.hpp"
# include <map>

class	HttpResponse
{
	public:
		explicit HttpResponse(Http::StatusCode code = Http::OK);

		void	setStatus(Http::StatusCode code);
		void	setHeader(const std::string& key, const std::string& value);
		void	setBody(const std::string& body, const std::string& contentType = "text/html");
		void	clearBody(void);

		// Used for HEAD responses: strips the body but keeps every header
		// (including Content-Length) exactly as a GET would have sent it.
		void	dropBodyForHead(void);

		bool				hasHeader(const std::string& key) const;
		std::string			getHeader(const std::string& key) const;
		Http::StatusCode	status(void) const;
		std::string			toString(void) const;

	private:
		Http::StatusCode					_status;
		std::map<std::string, std::string>	_headers;
		std::string							_body;
		bool								_omitBody;
};

#endif  // HttpResponse_HPP
