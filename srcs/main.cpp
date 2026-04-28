#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "socket/Socket.hpp"
#include "socket/SocketServer.hpp"
#include "socket/SocketClient.hpp"
#include "server/EventLoop.hpp"

int	main(void)
{
	EventLoop	loop;

	loop.run();
	return 0;
}
