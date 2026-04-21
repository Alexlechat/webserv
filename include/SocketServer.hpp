#ifndef SOCKETSERVER_HPP
# define SOCKETSERVER_HPP

#include <netinet/in.h>

#include "Socket.hpp"

class SocketServer : public Socket
{
    private:
        struct sockaddr_in  _address;

    public:
        SocketServer();
        ~SocketServer();

        void    bindSocket();
        void    listenSocket(int maxConnections);
};

#endif