#ifndef SOCKETCLIENT_HPP
# define SOCKETCLIENT_HPP

#include <string>
#include <sys/socket.h>

#include "socket/Socket.hpp"

class SocketClient : public Socket
{
    protected:
		std::string		_recv_buf;
		std::string		_send_buf;

    public:
        SocketClient(int fd);
        ~SocketClient(void);

        std::string&	getRecvBuf();
        std::string&	getSendBuf();
};

#endif
