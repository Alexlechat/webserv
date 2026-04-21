#ifndef SOCKET_HPP
# define SOCKET_HPP

class Socket
{
    private:
        int _fd;

    public:
        Socket();
        ~Socket();

        int getFd();
};

#endif