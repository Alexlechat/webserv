#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "Socket.hpp"
#include "SocketServer.hpp"

int	main(void)
{
	try
	{
		SocketServer	socketServer;

		socketServer.bindSocket();
		socketServer.listenSocket(10);
		std::cout << "listening on port 8080..." << std::endl;


		//ACCEPT CONNECTION
		int	client_fd = accept(socketServer.getFd(), NULL, NULL);
		if (client_fd > 0)
			std::cout << "client connected" << std::endl;

		//READ CLIENT
		char	buffer[1024];
		std::memset(buffer, 0, sizeof(buffer));
		if (recv(client_fd, buffer, sizeof(buffer), 0) != 0)
			std::cout << "received:\n" << buffer << std::endl;

		

		//SEND HTTP RESPONSE
		std::string	response =
			"HTTP/1.1 200 OK\r\n"
			"Content-length: 13\r\n"
			"\r\n"
			"Hello World!";
		send(client_fd, response.c_str(), response.size(), 0);

		close(client_fd);

	}

	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}


	return 0;
}
