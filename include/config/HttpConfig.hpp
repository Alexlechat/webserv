#ifndef HttpConfig_HPP
# define HttpConfig_HPP

# include "config/Config.hpp"
# include "config/ServerConfig.hpp"

struct	HttpConfig : public Config
{
	std::vector<ServerConfig>				servers;
};

#endif  // HttpConfig_HPP
