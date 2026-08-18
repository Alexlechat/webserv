#ifndef LocationConfig_HPP
# define LocationConfig_HPP

# include <set>
# include <cstdlib>
# include <stdexcept>

# include "config/Config.hpp"
#include "http/Http.hpp"
#include "utils/Utils.hpp"

struct	LocationConfig : public Config
{
	std::string					path;
	Directive<bool>				autoindex;
	Directive<std::string>		upload_store;
	Directive<std::string>		upload_return;

	std::set<std::string>		methods;
	std::pair<int, std::string>	redirect;

	void	setPath(const std::string& new_path)
	{
		path = new_path;
	}

	void	setAutoindex(const std::string& new_autoindex)
	{
		if (new_autoindex != "on" && new_autoindex != "off")
			throw std::invalid_argument("Invalid autoindex parameter : " + new_autoindex);

		autoindex.set(new_autoindex == "on");
	}

	void	setUploadStore(const std::string& new_upload_store)
	{
		if (!Utils::dir_exists(new_upload_store))
			throw std::invalid_argument("Invalid upload_store parameter : "
					+ new_upload_store + ". Directory does not exist");

		upload_store.set(new_upload_store);
	}

	void	setUploadReturn(const std::string& new_upload_return)
	{
		if (!Utils::dir_exists(new_upload_return))
			throw std::invalid_argument("Invalid upload_return parameter : "
					+ new_upload_return + ". Directory does not exist");

		upload_return.set(new_upload_return);
	}

	void	setMethods(const std::string& new_method)
	{
		
		if (Http::strToMethod(new_method) == Http::UNKNOWN_METHOD)
			throw std::invalid_argument("Invalid method argument" + new_method);

		methods.insert(new_method);
	}

	void	setRedirects(const std::string& new_redirect_code, const std::string& new_redirect_url)
	{
		char*	end;
		int		code = std::strtol(new_redirect_code.c_str(), &end, 10);

		if (*end != '\0')
			throw std::invalid_argument("Invalid redirect code :" + new_redirect_code);

		if (code < 300 || code > 308)
			throw std::out_of_range("Redirect code out of range :" + new_redirect_code);

		redirect = std::make_pair(code, new_redirect_url);
	}
};

#endif  // LocationConfig_HPP
