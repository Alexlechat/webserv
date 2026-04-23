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

        struct sockaddr_in  getAddress();

        void    bindSocket();
        void    listenSocket(int maxConnections);
        int     acceptClient();
};

#endif