#ifndef Client_HPP
# define Client_HPP

# include <string>

# include "socket/SocketClient.hpp"

class	Client : public SocketClient
{
	private:
		std::string	_sendBuffer;

		std::string	_parsePathFromRequest(void) const;
		std::string	_readFile(const std::string& filepath) const;

	public:
		Client(int fd);
		~Client(void);

		std::string& getRecvBuf();
        std::string& getSendBuf();

		bool	isRequestComplete(void) const;
		void	buildResponse(void);

};

#endif
