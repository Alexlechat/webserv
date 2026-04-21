#ifndef Client_HPP
# define Client_HPP

# include <string>

class	Client
{
	public:
		Client(int fd);
		~Client(void);

		int				getFd(void) const;
		std::string&	getRecvBuf(void);
		std::string&	getSendBuf(void);

		bool	isRequestComplete(void) const;
		void	buildResponse(void);

	private:
		int			_fd;
		std::string	_recv_buf;
		std::string	_send_buf;

		std::string	_parsePathFromRequest(void) const;
		std::string	_readFile(const std::string& filepath) const;
};

#endif  // Client_HPP
