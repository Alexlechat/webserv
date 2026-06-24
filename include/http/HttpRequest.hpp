#ifndef HttpRequest_HPP
# define HttpRequest_HPP

# include "http/Http.hpp"
# include <string>
# include <map>

class	HttpRequest
{
	public:
		enum	ParseState
		{
			PARSING_REQUEST_LINE,
			PARSING_HEADERS,
			PARSING_BODY,
			PARSING_COMPLETE,
			PARSING_ERROR
		};

		std::string							method;
		Http::Method						methodEnum;
		std::string							path;
		std::string							query;
		std::string							version;
		std::map<std::string, std::string>	headers;
		std::string							body;

		HttpRequest(void);

		ParseState	feed(const std::string& chunk);
		ParseState	state(void) const;
		bool		isComplete(void) const;
		bool		hasError(void) const;
		void		reset(void);

	private:
		std::string	_buffer;
		ParseState	_state;
		size_t		_contentLenght;
		bool		_chunked;

		ParseState	_tryParseRequestLine(void);
		ParseState	_tryParseHeaders(void);
		ParseState	_tryParseBody(void);
		ParseState	_tryParseChunkedBody(void);

		HttpRequest(const HttpRequest&);
		HttpRequest&	operator=(const HttpRequest&);
};

#endif  // HttpRequest_HPP
