#ifndef Config_HPP
# define Config_HPP

# include <map>
# include <string>
# include <limits>
# include <cstddef>
# include <cstdlib>
# include <stdexcept>

# include "config/FileLoggerConfig.hpp"
#include "utils/Utils.hpp"

template <typename T>
struct	Directive
{
	bool	specified;
	T		value;

	Directive(void) : specified(false), value() {}
	explicit Directive(const T& val) : specified(true), value(val) {}

	void	set(const T& new_value)
	{
		value = new_value;
		specified = true;
	}

	operator	const T&(void) const { return value; }
	operator	T&(void) { return value; }

	T*			operator->(void) { return &value; }
	const T*	operator->(void) const { return &value; }
};

template <typename T>
inline std::ostream&	operator<<(std::ostream& os, const Directive<T>& d)
{
	return os << d.value;
}

template <typename T>
inline void	inherit(Directive<T>& child, const Directive<T>& parent)
{
	if (!child.specified && parent.specified)
		child = parent;
}

template <typename K, typename V>
inline void	inheritMap(std::map<K, V>& child, const std::map<K, V>& parent)
{
	typename std::map<K, V>::const_iterator	it;

	for (it = parent.begin(); it != parent.end(); ++it)
	{
		if (child.find(it->first) == child.end())
			child[it->first] = it->second;
	}
}

struct	Config
{
	Directive<FileLoggerConfig>				error_log_config;
	Directive<FileLoggerConfig>				access_log_config;

	Directive<std::string>					root;
	Directive<std::string>					index;
	Directive<size_t>						client_max_body_size;

	std::map<unsigned short, std::string>	error_pages;
	std::map<std::string, std::string>		cgi_extensions;

	void	setRoot(const std::string& new_root)
	{
		if (!Utils::dir_exists(new_root))
			throw std::invalid_argument("Invalid root parameter : " + new_root);

		root.set(new_root);
	}

	void	setIndex(const std::string& new_index)
	{
		index.set(new_index);
	}

	void	setClientMaxBodySize(const std::string& new_cmbs)
	{
		char*			end;
		unsigned long	val = std::strtoul(new_cmbs.c_str(), &end, 10);

		if (*end != '\0')
			throw std::invalid_argument("Invalid client_max_body_size: " + new_cmbs);

		if (val > std::numeric_limits<size_t>::max())
			throw std::out_of_range("client_max_body_size out of range: " + new_cmbs);

		client_max_body_size.set(val);
	}

	void	setErrorPages(const std::string& new_code, const std::string& new_page)
	{
		char*	end;
		long	val = std::strtol(new_code.c_str(), &end, 10);

		if (*end != '\0')
			throw std::invalid_argument("Invalid error page code: " + new_code);

		if (val < 100 || val > 599)
			throw std::out_of_range("Error page code out of range: " + new_code);

		error_pages[static_cast<unsigned short>(val)] = new_page;
	}

	void	setCgiExtensions(const std::string& extension, const std::string& interpreter)
	{
		cgi_extensions[extension] = interpreter;
	}
};

#endif
