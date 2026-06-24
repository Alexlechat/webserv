#ifndef LocationConfig_HPP
# define LocationConfig_HPP

# include <map>
# include <set>
#include <stdexcept>
# include <string>
# include <utility>
# include <cstdlib>

struct	LocationConfig
{
	std::string								path;
	std::string								root;
	std::string								index;
	std::string								upload_store;
	bool									autoindex;
	std::set<std::string>					methods;
	std::pair<int, std::string>				redirect;
	std::map<std::string, std::string>		cgi_extensions;

	LocationConfig(void) {}

	void	setPath(std::string new_path) { path = new_path; }
	void	setRoot(std::string new_root) { root = new_root; }
	void	setIndex(std::string new_index) { index = new_index; }
	void	setUploadStore(std::string new_upload_store) { upload_store = new_upload_store; }
	void	setAutoindex(std::string new_autoindex) { autoindex = new_autoindex == "on"; }

	void	setMethods(std::string new_method) { methods.insert(new_method); }
	void	setRedirects(std::string new_redirect_code, std::string new_redirect_url)
	{
		char*	end;
		int		code = std::strtol(new_redirect_code.c_str(), &end, 10);

		if (*end != '\0')
			throw std::invalid_argument("Invalid redirect code :" + new_redirect_code);

		if (code < 300 || code > 308)
			throw std::out_of_range("Redirect code out of range :" + new_redirect_code);

		redirect = std::make_pair(code, new_redirect_url);
	}
	void	setCgiExtensions(std::string extension, std::string interpreter) { cgi_extensions[extension] = interpreter; }
};

#endif  // LocationConfig_HPP
