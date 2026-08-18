#ifndef SOCKETCLIENT_HPP
# define SOCKETCLIENT_HPP

#include <string>
#include <netinet/in.h>
#include <sys/socket.h>

#include "socket/Socket.hpp"

class SocketClient : public Socket
{
	private:
		struct sockaddr_in	_address;

    protected:
		std::string			_recv_buf;
		std::string			_send_buf;

    public:
        SocketClient(int fd);
        ~SocketClient(void);

        std::string&		getRecvBuf(void);
        std::string&		getSendBuf(void);

		void				setSockAddr(const struct sockaddr_in& addr);
		const struct sockaddr_in&	getSockAddr(void) const;
};

#endif
