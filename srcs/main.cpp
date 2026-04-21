#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "Socket.hpp"

int	main(void)
{
	//CREATE SOCKET
	// int	socket._server_fd = socket(AF_INET, SOCK_STREAM, 0);
	// if (socket._server_fd < 0)
	// {
	// 	std::cerr << "socket() failed" << std::endl;
	// 	return (1);
	// }

	Socket socket;

	//LINKED TO ADRESS AND PORT
	struct sockaddr_in	address;
	std::memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(8080);
	address.sin_addr.s_addr = INADDR_ANY;

	if (bind(socket.getFd(), (struct sockaddr*)&address, sizeof(address)) < 0)
	{
		std::cerr << "bind() failed" << std::endl;
		return (1);
	}

	//LISTEN (max 10)
	listen(socket.getFd(), 10);
	std::cout << "listening on port 8080 ..." << std::endl;

	//ACCEPT CONNECTION
	int	client_fd = accept(socket.getFd(), NULL, NULL);
	std::cout << "client connected" << std::endl;

	//READ CLIENT
	char	buffer[1024];
	std::memset(buffer, 0, sizeof(buffer));
	recv(client_fd, buffer, sizeof(buffer), 0);
	std::cout << "received:\n" << buffer << std::endl;

	//SEND HTTP RESPONSE
	std::string	response =
		"HTTP/1.1 200 OK\r\n"
		"Content-length: 13\r\n"
		"\r\n"
		"Hello World!";
	send(client_fd, response.c_str(), response.size(), 0);

	close(client_fd);


	return 0;
}
