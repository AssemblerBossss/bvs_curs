#include "Server.h"
#include <cstring>
#include <ranges>


namespace Net {


// Для преобразования из строки
        Protocol parse_protocol(std::string_view str) {
            if (str == "tcp" || str == "TCP") return Protocol::TCP;
            if (str == "udp" || str == "UDP") return Protocol::UDP;
            throw std::invalid_argument("Invalid protocol: " + std::string(str));
        }

        /// Преобразует Protocol в строку
        /// @param protocol Протокол
        /// @return Строковое представление ("TCP" или "UDP")
        std::string to_string(Protocol protocol) {
            switch (protocol) {
                case Protocol::TCP:
                    return "TCP";
                case Protocol::UDP:
                    return "UDP";
            }
            return "UNKNOWN";
        }

        /// Конструктор сервера
        /// @param port Порт, на котором будет работать сервер
        /// @param protocol Протокол (TCP или UDP)
        Server::Server(uint16_t port, Protocol protocol)
        : port_(port), protocol_(protocol), is_running_(false), server_socket_(-1) {

            if (port == 0 || port > 65535) {
                throw std::invalid_argument("Invalid port number: " + std::to_string(port));
            }
        }

        /// Деструктор сервера, автоматически останавливает сервер при уничтожении объекта
        Server::~Server() {
            stop();
        }

/*void Net::Server:read_config*/

        /// Настраивает сокет в соответствии с выбранным протоколом
        void Server::setup_socket() {
            // 1. Создание сокета
            int socket_type = (protocol_ == Protocol::TCP) ? SOCK_STREAM : SOCK_DGRAM;

            server_socket_ = socket(AF_INET, socket_type, 0);
            if (server_socket_ < 0) {
                throw std::runtime_error("Socket creation failed: " + std::string(strerror(errno)));
            }

            // 2. Установка опции SO_REUSEADDR
            int opt = 1;
            if (setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0) {
                close(server_socket_);
                throw std::runtime_error("Setsockopt(SO_REUSEADDR) failed: " + std::string(strerror(errno)));
            }

            // 3. Настройка адреса сервера
            struct sockaddr_in server_addr{};
            server_addr.sin_family = AF_INET;                 // IPv4
            server_addr.sin_addr.s_addr = INADDR_ANY;         // Любой локальный адрес
            server_addr.sin_port = htons(port_);     // Порт (big-endian)

            // 4. Привязка сокета
            if (bind(server_socket_, reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr)) != 0) {
                close(server_socket_);
                throw std::runtime_error("Bind failed: " + std::string(strerror(errno)));
            }

            // 5. Прослушивание входящих соединений (только для TCP)
            if (protocol_ == Protocol::TCP) {
                const int backlog = 128;
                if (listen(server_socket_, backlog) != 0) {
                    close(server_socket_);
                    throw std::runtime_error("Listen failed: " + std::string(strerror(errno)));
                }
            }
        }

        /// Запускает сервер и начинает прослушивание клиентов
        void Server::start() {
            if (is_running_) return;

            setup_socket();
            is_running_ = true;

            std::cout << std::uppercase
                      << to_string(protocol_) << " server started on port: "
                      << port_ << std::endl;

            // Запуск соответствующего слушателя в отдельном потоке
            if (protocol_ == Protocol::TCP) {
                listener_thread_ = std::make_unique<std::thread>(&Server::tcp_listen, this);
            } else {
                listener_thread_ = std::make_unique<std::thread>(&Server::udp_listen, this);
            }
        }

        /// Останавливает сервер и освобождает все ресурсы
        void Server::stop() {
            if (!is_running_) return;

            is_running_ = false;

            close(server_socket_);
            server_socket_ = -1;

            if (listener_thread_ && listener_thread_->joinable()) {
                listener_thread_->join();
            }

            for (int client: tcp_clients_) {
                close(client);
            }
            tcp_clients_.clear();

            udp_clients_.clear();

            std::cout << "Server stopped.\n";
        }

        /// Основной цикл прослушивания TCP-подключений
        void Server::tcp_listen() {
            while (is_running_) {
                sockaddr client_addr;
                socklen_t client_len = sizeof(client_addr);

                int client_socket = accept(server_socket_,
                                           reinterpret_cast<sockaddr *>(&client_addr),
                                           &client_len
                );

                if (client_socket < 0) {
                    if (is_running_) std::cerr << "Accept failed: " << strerror(errno) << std::endl;
                    continue;
                }
                tcp_clients_.push_back(client_socket);
                std::thread(&Server::handle_tcp_client, this, client_socket, client_addr);
            }
        }

        /// Обрабатывает подключение одного TCP-клиента
        /// @param client_socket Дескриптор клиента
        /// @param client_addr Адрес клиента
        void Server::handle_tcp_client(int client_socket, sockaddr_in client_addr) {
            char buffer[1024];
            while (is_running_) {
                ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
                if (bytes_received <= 0) break;

                buffer[bytes_received] = '\0';
                broadcast_tcp(buffer, client_socket);
            }
            close(client_socket);

            tcp_clients_.erase(std::remove(tcp_clients_.begin(), tcp_clients_.end(), client_socket), tcp_clients_.end());
        }

        /// Отправляет сообщение всем TCP-клиентам, кроме отправителя
        /// @param message Сообщение для отправки
        /// @param sender_socket Сокет отправителя
        void Server::broadcast_tcp(const std::string &message, int sender_socket) {
            for (int client: tcp_clients_) {
                if (client != sender_socket) {
                    send(client, message.c_str(), message.length(), 0);
                }
            }
        }










};
