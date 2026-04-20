#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>

int	main(void)
{
	int	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
	{
		std::cerr << "Socket creation failed\n";
		return 1;
	}

	struct sockaddr_in	server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(8080);

	if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		std::cerr << "Server Bind failed\n";
		return 1;
	}

	if (listen(server_fd, 5) < 0)
	{
		std::cerr << "Server Listen failed\n";
		return 1;
	}

	while (42)
	{
		struct sockaddr_in	client_addr;

		socklen_t	client_addr_len = sizeof(client_addr);
		int	client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
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
			std::ifstream	file("index.html");
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
			std::ifstream	file("test");
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

	close(server_fd);
	return 0;
}
