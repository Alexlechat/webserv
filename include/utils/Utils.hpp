#ifndef Utils_HPP
# define Utils_HPP

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
}

#endif  // Utils_HPP
