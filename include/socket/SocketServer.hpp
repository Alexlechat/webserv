#ifndef SOCKETSERVER_HPP
# define SOCKETSERVER_HPP

#include <netinet/in.h>

#include "Socket.hpp"

class SocketServer : public Socket
{
    private:
        struct sockaddr_in	_address;

    public:
		SocketServer(uint16_t port);
        ~SocketServer(void);

        struct sockaddr_in  getAddress();

        void    bindSocket(void);
        void    listenSocket(int maxConnections);
        int     acceptClient(void);
};

#endif
