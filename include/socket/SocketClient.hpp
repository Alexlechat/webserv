#ifndef SOCKETCLIENT_HPP
# define SOCKETCLIENT_HPP

#include <string>
#include <sys/socket.h>

#include "Socket.hpp"

class SocketClient : public Socket
{
    private:
        std::string _bufferRequest;   

    public:
        SocketClient(int fd);
        ~SocketClient();

        std::string getBufferRequest();

        int    receiveData();
        int    sendData(const std::string& response);

};

#endif