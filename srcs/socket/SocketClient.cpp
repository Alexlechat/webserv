#include "socket/SocketClient.hpp"

SocketClient::SocketClient(int fd) : Socket(fd) {}
SocketClient::~SocketClient(void) {}

std::string& SocketClient::getRecvBuf(void) { return _recv_buf; }
std::string& SocketClient::getSendBuf(void) { return _send_buf; }