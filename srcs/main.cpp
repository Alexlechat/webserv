#include <iostream>
#include <exception>
#include "server/EventLoop.hpp"

# define DEFAULT_CONFIG_PATH "./www/config.cfg"

int	main(int argc, char** argv)
{
	if (argc > 2)
	{
		std::cerr << "Usage: " << argv[0] << " [configuration file]" << std::endl;
		return 1;
	}

	std::string	configPath = (argc == 2) ? argv[1] : DEFAULT_CONFIG_PATH;

	try
	{
		EventLoop	loop(configPath);
		return loop.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Fatal: " << e.what() << std::endl;
		return 1;
	}
}
