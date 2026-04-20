/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fdeleard <fdeleard@student.42lyon.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/20 12:29:41 by fdeleard          #+#    #+#             */
/*   Updated: 2026/04/20 12:42:10 by fdeleard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unistd.h>

#include "Server.hpp"
#include <sys/socket.h>
#include <netinet/in.h>

int	main(void)
{
	Server	server;

	while (42)
	{
		struct sockaddr_in	client_addr;

		socklen_t	client_addr_len = sizeof(client_addr);
		int	client_fd = accept(server.getServerFd(), (struct sockaddr *)&client_addr, &client_addr_len);
		if (client_fd < 0)
		{
			std::cerr << "Client Accept failed\n";
			continue ;
		}

		char	buffer[1024] = {0};

		read(client_fd, buffer, sizeof(buffer) - 1);

		std::stringstream	response;
		std::string	path = "/";
		std::string	request(buffer);

		std::cout << "REQUEST: " << request << "\n";

		if (request.find("GET / ") != std::string::npos)
			path = "/";
		else if (request.find("GET /") != std::string::npos)
		{
			size_t	start = request.find("GET /") + 5;
			size_t	end = request.find(" ", start);
			path = request.substr(start, end - start);
		}

		if (path == "/")
		{
			std::ifstream	file("www/index.html");
			std::string		buffer;
			std::stringstream	test;

			while (std::getline(file, buffer))
				test << buffer;

			response << "HTTP/1.1 200 OK\n";
			response << "Content-Type: text/html; charset=UTF-8\n";
			response << "Content-Lenght: " << test.str().length();
			response << "\n\n";

			response << test.str();
			response << "\r\n\r\n";

			std::cout << "RESPONSE: \n" << response.str() << "\n";
			file.close();
		}
		else if (path == "test")
		{
			std::ifstream	file("www/test");
			std::string		buffer;
			std::stringstream	test;

			while (std::getline(file, buffer))
				test << buffer;

			response << "HTTP/1.1 200 OK\n";
			response << "Content-Type: text/html; charset=UTF-8\n";
			response << "Content-Lenght: " << test.str().length();
			response << "\n\n";

			response << test.str();
			response << "\r\n\r\n";

			std::cout << "RESPONSE: \n" << response.str() << "\n";
			file.close();
		}
		else
			response << "HTTP/1.1 404 Not Found\r\n\r\n";

		write(client_fd, response.str().c_str(), response.str().length());
		close(client_fd);
	}

	return 0;
}
