#ifndef SOCKETSERVER_HPP
# define SOCKETSERVER_HPP

#include <string>
#include <netinet/in.h>

#include "Socket.hpp"

class SocketServer : public Socket
{
    private:
        struct sockaddr_in	_address;

    public:
		SocketServer(uint16_t port, const std::string& host = "");
        ~SocketServer(void);

        struct sockaddr_in  getAddress();

        void    bindSocket(void);
        void    listenSocket(int maxConnections);
};

#endif
