#include "server/EventLoop.hpp"
#include "config/ConfigParser.hpp"

#include <iostream>
#include <exception>

int	main(void)
{
	try
	{
		ConfigParser	config("www/config");
	}
	catch (const std::exception& e) { std::cerr << e.what() << std::endl; }

	// EventLoop	loop;
	//
	// loop.run();
	return 0;
}
