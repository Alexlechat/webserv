#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "server/EventLoop.hpp"

int	main(void)
{
	EventLoop	loop;

	loop.run();
	return 0;
}
