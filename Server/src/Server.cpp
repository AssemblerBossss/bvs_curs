#include "Server.h"

Net::Server::Server(uint16_t port, const std::string &protocol)
    : port_(port), protocol_(protocol), is_running_(false), server_socket_(-1) {}
