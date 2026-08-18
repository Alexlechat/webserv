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

	std::string	normalizePath(const std::string& path);

	std::string	sanitizeFilename(const std::string& filename);

	std::string	ipv4ToString(unsigned int hostOrderAddr);

	bool		ipv4FromString(const std::string& dotted, unsigned int& hostOrderAddr);

	std::string	urlDecode(const std::string& s);
	std::string	htmlEscape(const std::string& s);

	bool		dir_exists(const std::string& path);
	bool		file_exists(const std::string& filepath);
};

#endif
