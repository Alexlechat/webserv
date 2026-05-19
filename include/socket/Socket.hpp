#ifndef SOCKET_HPP
# define SOCKET_HPP

class Socket
{
    private:
        int	_fd;

    public:
        Socket(void);
        Socket(int fd);
        ~Socket(void);

        int		getSocketFd(void) const;
		void	setNonBlocking(void) const;
};

#endif
