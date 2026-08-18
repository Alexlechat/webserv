#ifndef Http_HPP
# define Http_HPP

# include <string>

namespace	Http
{
	enum	Method { GET, POST, DELETE, PUT, HEAD, OPTIONS, UNKNOWN_METHOD };

	inline std::string	methodToStr(Method m)
	{
		switch (m)
		{
			case GET:		return "GET";
			case POST:		return "POST";
			case DELETE:	return "DELETE";
			case PUT:		return "PUT";
			case HEAD:		return "HEAD";
			case OPTIONS:	return "OPTIONS";
			default:		return "UNKNOWN";
		}
	}

	inline Method	strToMethod(const std::string& s)
	{
		if (s == "GET")			return GET;
		if (s == "POST")		return POST;
		if (s == "DELETE")		return DELETE;
		if (s == "PUT")			return PUT;
		if (s == "HEAD")		return HEAD;
		if (s == "OPTIONS")		return OPTIONS;
		return UNKNOWN_METHOD;
	}

	enum	StatusCode
	{
        CONTINUE				= 100,
        OK                  	= 200,
        CREATED             	= 201,
        NO_CONTENT          	= 204,
        MOVED_PERMANENTLY   	= 301,
        FOUND               	= 302,
		SEE_OTHER				= 303,
        NOT_MODIFIED        	= 304,
        BAD_REQUEST         	= 400,
        FORBIDDEN           	= 403,
        NOT_FOUND           	= 404,
        METHOD_NOT_ALLOWED  	= 405,
        REQUEST_TIMEOUT     	= 408,
        CONTENT_TOO_LARGE   	= 413,
        URI_TOO_LONG        	= 414,
        HEADERS_TOO_LARGE   	= 431,
        INTERNAL_SERVER_ERROR	= 500,
        NOT_IMPLEMENTED			= 501,
        BAD_GATEWAY         	= 502,
        SERVICE_UNAVAILABLE 	= 503
    };

    inline std::string	statusToReason(StatusCode code)
    {
        switch (code)
		{
            case CONTINUE:              return "Continue";
            case OK:                    return "OK";
            case CREATED:               return "Created";
            case NO_CONTENT:            return "No Content";
            case MOVED_PERMANENTLY:     return "Moved Permanently";
            case FOUND:                 return "Found";
			case SEE_OTHER:				return "See Other";
            case NOT_MODIFIED:          return "Not Modified";
            case BAD_REQUEST:           return "Bad Request";
            case FORBIDDEN:             return "Forbidden";
            case NOT_FOUND:             return "Not Found";
            case METHOD_NOT_ALLOWED:    return "Method Not Allowed";
            case REQUEST_TIMEOUT:       return "Request Timeout";
            case CONTENT_TOO_LARGE:     return "Content Too Large";
            case URI_TOO_LONG:          return "URI Too Long";
            case HEADERS_TOO_LARGE:     return "Request Header Fields Too Large";
            case INTERNAL_SERVER_ERROR: return "Internal Server Error";
            case NOT_IMPLEMENTED:       return "Not Implemented";
            case BAD_GATEWAY:           return "Bad Gateway";
            case SERVICE_UNAVAILABLE:   return "Service Unavailable";
            default:                    return "Unknown";
        }
    }

    static const char* const	CRLF        = "\r\n";
    static const char* const	HEADER_END  = "\r\n\r\n";
    static const char* const	VERSION     = "HTTP/1.1";
    static const char* const	SPACE_CHARS = " \t\r\n";
}

#endif
