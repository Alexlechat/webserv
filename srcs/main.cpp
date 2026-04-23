#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "Socket.hpp"
#include "SocketServer.hpp"
#include "SocketClient.hpp"

int	main(void)
{
	try
	{
		SocketServer	socketServer;
		int				client_fd;
		
		std::string	response =
			"HTTP/1.1 200 OK\r\n"
			"Content-length: 13\r\n"
			"\r\n"
			"Hello World!";

		socketServer.bindSocket();
		socketServer.listenSocket(10);
		std::cout << "listening on port 8080..." << std::endl;
		client_fd = socketServer.acceptClient();
		
		//READ CLIENT
		SocketClient	socketClient(client_fd);
		if (socketClient.receiveData() > 0)
			std::cout << "received:\n" << socketClient.getBufferRequest() << std::endl;

		//SEND HTTP RESPONSE
		socketClient.sendData(response);

		close(client_fd);

	}

	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}


	return 0;
}
