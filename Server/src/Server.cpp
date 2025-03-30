#include "Server.h"
#include <cstring>
#include <ranges>
#include <format>

Net::Server::Server(uint16_t port, const std::string &protocol)
        : port_(port), protocol_(protocol), is_running_(false), server_socket_(-1) {
}

/*void Net::Server:read_config*/

void Net::Server::setup_socket() {
    // Валидация protocol_
    if (protocol_ != "tcp" && protocol_ != "udp") {
        throw std::runtime_error("Invalid protocol: " + protocol_);
    }

    server_socket_ = socket(AF_INET, protocol_ == "tcp" ? SOCK_STREAM : SOCK_DGRAM, 0);

    if (server_socket_ < 0) {
        throw std::runtime_error("Socket creation failed: " + std::string(strerror(errno)));
    }

    // Установка опций сокета
    int opt = 1;
    if (setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        close(server_socket_);
        throw std::runtime_error("Setsockopt(SO_REUSEADDR) failed: " + std::string(strerror(errno)));
    }

    // Привязка сокета
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;             // IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY;     // Принимать соединения на все IP-адреса
    server_addr.sin_port = htons(port_); // Порт (в сетевом порядке байт)

    if (bind(server_socket_, (struct sockaddr *) &server_addr, sizeof(server_addr))) {
        close(server_socket_);
        throw std::runtime_error("Bind failed");
    }

    // Для TCP: переход в режим прослушивания
    if (protocol_ == "tcp") {
        const int backlog = 128; // Настраиваемый размер очереди
        if (listen(server_socket_, backlog) == -1) {
            close(server_socket_);
            throw std::runtime_error("Listen failed: " + std::string(strerror(errno)));
        }
    }
}


void Net::Server::start() {
    if (is_running_) return;

    setup_socket();
    is_running_ = true;

    std::string upper_protocol_ = protocol_;
    std::transform(upper_protocol_.begin(), upper_protocol_.end(), upper_protocol_.begin(), ::toupper);
    std::cout << upper_protocol_
              << " server started on port: "
              << port_ << std::endl;

    if (protocol_ == "tcp") {
        listener_thread_ = std::make_unique<std::thread>(&Server::tcp_listen, this);
    } else {
        listener_thread_ = std::make_unique<std::thread>(&Server::udp_listen, this);
    }
}