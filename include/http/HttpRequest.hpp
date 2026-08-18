#ifndef HttpRequest_HPP
# define HttpRequest_HPP

# include "http/Http.hpp"
# include <string>
# include <map>

// Upper bounds on the part of a request that has to be buffered before
// it can be understood. Without them a single client can make the
// server allocate without limit just by never sending the blank line
// that ends the head. nginx uses the same order of magnitude.
# define MAX_REQUEST_LINE_SIZE	8192
# define MAX_HEADERS_SIZE		8192

class	HttpRequest
{
	public:
		enum	ParseState
		{
			PARSING_REQUEST_LINE,
			PARSING_HEADERS,
			PARSING_BODY,
			PARSING_COMPLETE,
			PARSING_ERROR,
			PARSING_TOO_LARGE,		// body over client_max_body_size -> 413
			PARSING_URI_TOO_LONG,	// request line over the cap  -> 414
			PARSING_HEADERS_TOO_LARGE	// head over the cap     -> 431
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

		// 0 means unlimited. Must be set before feed() is first called
		// with a body-bearing request if enforcement is desired.
		void		setMaxBodySize(size_t maxBodySize);

	private:
		std::string	_buffer;
		ParseState	_state;
		size_t		_contentLenght;
		bool		_chunked;
		size_t		_maxBodySize;

		static bool	_isTerminal(ParseState state);

		ParseState	_tryParseRequestLine(void);
		ParseState	_tryParseHeaders(void);
		ParseState	_tryParseBody(void);
		ParseState	_tryParseChunkedBody(void);

		HttpRequest(const HttpRequest&);
		HttpRequest&	operator=(const HttpRequest&);
};

#endif  // HttpRequest_HPP
