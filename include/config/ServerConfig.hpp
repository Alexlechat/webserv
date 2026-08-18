#ifndef ServerConfig_HPP
# define ServerConfig_HPP

# include <vector>
# include <stdexcept>

# include "config/Config.hpp"
# include "config/LocationConfig.hpp"

struct	ServerConfig : public Config
{
	std::vector<LocationConfig>				locations;

	Directive<unsigned short>				port;
	Directive<std::string>					server_name;

	void	setPort(const std::string& new_port)
	{
		char*	end;
		long	val = std::strtol(new_port.c_str(), &end, 10);

		if (*end != '\0')
			throw std::invalid_argument("Invalid port: " + new_port);

		if (val < 1 || val > 65535)
			throw std::out_of_range("Port out of range: " + new_port);

		port.set(static_cast<unsigned short>(val));
	}

	void	setServerName(const std::string& new_server_name)
	{
		server_name.set(new_server_name);
	}
};

#endif  // ServerConfig_HPP
