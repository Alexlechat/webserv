#ifndef SOCKET_HPP
# define SOCKET_HPP

class	Socket
{
    public:
        Socket(void);
		Socket(int fd);
        ~Socket(void);

        int	getSocketFD(void) const;

    private:
        int _fd;
};

#endif
