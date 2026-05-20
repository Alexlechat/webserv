#ifndef ServerConfig_HPP
# define ServerConfig_HPP

# include <map>
# include <string>
# include <vector>
# include <cstdlib>

# include "config/FileLoggerConfig.hpp"
# include "config/LocationConfig.hpp"

struct	ServerConfig
{
	int							port;
	std::string					server_name;
	size_t						client_max_body_size;
	std::map<int, std::string>	error_pages;
	std::vector<LocationConfig>	locations;
	FileLoggerConfig					logConfig;

	ServerConfig(const ServerConfig& src) 
		: port(src.port), server_name(src.server_name), client_max_body_size(src.client_max_body_size), 
		  error_pages(src.error_pages), locations(src.locations), logConfig(src.logConfig) {}

	ServerConfig(FileLoggerConfig globalLogConfig) : port(80), client_max_body_size(1048576), logConfig(globalLogConfig) {}

	void	setPort(std::string new_port) { port = std::atoi(new_port.c_str()); }
	void	setServerName(std::string new_server_name) { server_name = new_server_name; }
	void	setClientMaxBodySize(std::string new_client_mbs) { client_max_body_size = static_cast<size_t>(std::atoi(new_client_mbs.c_str())); }
	void	setErrorPages(std::string new_code, std::string new_page) { int	code = std::atoi(new_code.c_str()); error_pages[code] = new_page; }
	void	setLocations(LocationConfig new_location) { locations.push_back(new_location); }
};

#endif  // ServerConfig_HPP
