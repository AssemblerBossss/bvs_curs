#include "Server.h"

Net::Server::Server(uint16_t port, const std::string &protocol)
    : port_(port), protocol_(protocol), is_running_(false), server_socket_(-1) {
}

void Net::Server::setup_socket() {
    server_socket_ = socket(AF_INET, protocol_ == "tcp" ? SOCK_STREAM : SOCK_DGRAM, 0);

    if (server_socket_ < 0) {
        throw std::runtime_error("Socket creation failed");
    }

    int opt;
    if (setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        throw std::runtime_error("Setsockopt failed");
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;             // IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY;     // Принимать соединения на все IP-адреса
    server_addr.sin_port = htons(port_); // Порт (в сетевом порядке байт)

    if (bind(server_socket_, (struct sockaddr*)&server_addr, sizeof(server_addr))) {
        throw std::runtime_error("Bind failed");
    }

    if (protocol_ == "tcp" && listen(server_socket_, SOMAXCONN) == -1) {
        throw std::runtime_error(
                "Listen failed: " +
                std::string(strerror(errno)) +  // Добавляем текст системной ошибки
                " (errno: " + std::to_string(errno) + ")"
        );
    }
}