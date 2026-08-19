#ifndef SOCKET_HPP
# define SOCKET_HPP

class	Socket
{
	private:
        int	_fd;

    public:
        Socket(void);
        virtual ~Socket(void);

		Socket(int fd) : _fd(fd) {}

        int		getSocketFd(void) const;
		void	setNonBlocking(void) const;
};

#endif
