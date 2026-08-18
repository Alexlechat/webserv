#ifndef Utils_HPP
# define Utils_HPP

# include <string>
# include <sstream>

namespace Utils
{
	template <typename T>
	std::string	toString(T n)
	{
		 std::stringstream	ss;

		 ss << n;
		 return ss.str();
	}

	template <typename T>
	std::string	fdInBrackets(T n)
	{
		std::string	fd = toString(n);

		return "[" + fd + "]";
	}

	std::string	strToLower(const std::string& s);
	std::string	strToUpper(const std::string& s);
	std::string	readFile(const std::string& filepath);
	std::string	trim(const std::string& s, const char* pattern);

	// Collapses "." and ".." segments in a '/'-rooted path so that a
	// request path can never climb above the location root via "../".
	// Returns "" if the path tries to escape above the root.
	std::string	normalizePath(const std::string& path);

	// Strips any directory components and rejects "." / ".." so an
	// uploaded filename can never be used to write outside upload_store.
	std::string	sanitizeFilename(const std::string& filename);

	// Formats an IPv4 address (already in host byte order, e.g. the
	// result of ntohl()) as a dotted-quad string. Deliberately hand
	// rolled: inet_ntoa()/inet_ntop() are not on the subject's allowed
	// external-function list.
	std::string	ipv4ToString(unsigned int hostOrderAddr);

	bool		dir_exists(const std::string& path);
	bool		file_exists(const std::string& filepath);
};

#endif  // Utils_HPP
