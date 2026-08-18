#include "socket/SocketClient.hpp"

SocketClient::SocketClient(int fd) : Socket(fd) {}
SocketClient::~SocketClient(void) {}

std::string&		SocketClient::getRecvBuf(void) { return _recv_buf; }
std::string& 		SocketClient::getSendBuf(void) { return _send_buf; }

void						SocketClient::setSockAddr(const struct sockaddr_in& addr) { _address = addr; }
const struct sockaddr_in&	SocketClient::getSockAddr(void) const { return _address; }
