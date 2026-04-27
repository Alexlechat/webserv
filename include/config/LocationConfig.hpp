#ifndef LocationConfig_HPP
# define LocationConfig_HPP

# include <cstdlib>
# include <map>
# include <string>
# include <vector>

struct	LocationConfig
{
	std::string							path;
	std::string							root;
	std::string							index;
	std::vector<std::string>			methods;
	bool								autoindex;
	std::string							upload_store;
	int									redirect_code;
	std::string							redirect_url;
	std::map<std::string, std::string>	cgi_extensions;

	LocationConfig(void) : autoindex(false), redirect_code(0) {}

	void	setPath(std::string new_path) { path = new_path; }
	void	setRoot(std::string new_root) { root = new_root; }
	void	setIndex(std::string new_index) { index = new_index; }
	void	setMethods(std::string new_method) { methods.push_back(new_method); }
	void	setAutoindex(std::string new_autoindex) { autoindex = new_autoindex == "on"; }
	void	setUploadStore(std::string new_upload_store) { upload_store = new_upload_store; }
	void	setRedirect(std::string new_redirect_code, std::string new_redirect_url) { redirect_code = std::atoi(new_redirect_code.c_str()); redirect_url = new_redirect_url; }
	void	setCgiExtensions(std::string extension, std::string interpreter) { cgi_extensions[extension] = interpreter; }
};

#endif  // LocationConfig_HPP
