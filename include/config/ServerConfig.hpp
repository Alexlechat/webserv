#ifndef ServerConfig_HPP
# define ServerConfig_HPP

# include <map>
# include <string>
# include <vector>
# include <limits>
# include <cstddef>
# include <stdexcept>

# include "config/FileLoggerConfig.hpp"
# include "config/LocationConfig.hpp"

struct	ServerConfig
{
	unsigned short							port;
	std::string								server_name;
	size_t									client_max_body_size;
	std::map<unsigned short, std::string>	error_pages;
	std::vector<LocationConfig>				locations;
	FileLoggerConfig						errorLogConfig;
	FileLoggerConfig						accessLogConfig;
	

	ServerConfig(void) : errorLogConfig(FileLoggerConfig()) {}

	void	setPort(std::string new_port)
	{
		char*	end;
		long	val = std::strtol(new_port.c_str(), &end, 10);

		if (*end != '\0')
			throw std::invalid_argument("Invalid port: " + new_port);

		if (val < 1 || val > 65535)
			throw std::out_of_range("Port out of range: " + new_port);

		port = static_cast<unsigned short>(val);
	}

	void	setServerName(std::string new_server_name) { server_name = new_server_name; }

	void	setClientMaxBodySize(std::string new_client_mbs)
	{
		char*			end;
		unsigned long	val = std::strtoul(new_client_mbs.c_str(), &end, 10);

		if (*end != '\0')
			throw std::invalid_argument("Invalid client_max_body_size: " + new_client_mbs);

		if (val > std::numeric_limits<size_t>::max())
			throw std::out_of_range("Port out of range: " + new_client_mbs);

		client_max_body_size = static_cast<size_t>(val);
	}

	void	setErrorPages(std::string new_code, std::string new_page)
	{
		char*			end;
		long			val = std::strtol(new_code.c_str(), &end, 10);

		if (*end != '\0')
			throw std::invalid_argument("Invalid error page code: " + new_code);

		if (val < 100 || val > 599)
			throw std::out_of_range("Port out of range: " + new_code);

		error_pages[static_cast<unsigned short>(val)] = new_page;

	}

	void	setLocations(LocationConfig new_location) { locations.push_back(new_location); }
};

#endif  // ServerConfig_HPP
