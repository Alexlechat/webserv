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
};

#endif  // Utils_HPP
