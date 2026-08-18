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
	Directive<std::string>					host;
	Directive<std::string>					server_name;

	void	setListen(const std::string& listen_value)
	{
		std::string	portStr = listen_value;
		size_t		colon = listen_value.rfind(':');

		if (colon != std::string::npos)
		{
			host.set(listen_value.substr(0, colon));
			portStr = listen_value.substr(colon + 1);
		}

		char*	end;
		long	val = std::strtol(portStr.c_str(), &end, 10);

		if (*end != '\0')
			throw std::invalid_argument("Invalid port: " + listen_value);

		if (val < 1 || val > 65535)
			throw std::out_of_range("Port out of range: " + listen_value);

		port.set(static_cast<unsigned short>(val));
	}

	void	setServerName(const std::string& new_server_name)
	{
		server_name.set(new_server_name);
	}
};

#endif
