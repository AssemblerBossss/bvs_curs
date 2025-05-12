#include "Server.h"
#include <cstring>
#include <ranges>


namespace Net {

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
            std::cout << "Server started on port: " << port_ << " using " << to_string(protocol_) << " protocol." << std::endl;

        }

        /// Запускает сервер и начинает прослушивание клиентов
        void Server::start() {
            if (is_running_) return;

            setup_socket();
            is_running_ = true;
            std::cout << "Server is now running..." << std::endl;

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
                sockaddr_in client_addr;
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
                std::thread client_thread(&Server::handle_tcp_client, this, client_socket, client_addr);
                client_thread.detach(); // Отсоединяем поток, чтобы он мог работать независимо
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

                // Логирование полученного сообщения
                std::cout << "Received message from "
                          << inet_ntoa(client_addr.sin_addr) << ": " << buffer << std::endl;

                // Отправляем подтверждение клиенту
                std::string ack_message = "OK";
                send(client_socket, ack_message.c_str(), ack_message.length(), 0);

                // Рассылаем сообщение другим клиентам
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

    /// Основной цикл прослушивания UDP-cooбщений
        void Server::udp_listen() {
            char buffer[1024];
            sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);

            while (is_running_) {
                ssize_t bytes_received = recvfrom(server_socket_, buffer, sizeof(buffer) - 1, 0,
                                                  reinterpret_cast<sockaddr *>(&client_addr), &client_len);
                if (bytes_received > 0) {
                    buffer[bytes_received] = '\n';
                    broadcast_udp(buffer, &client_addr);

                }
            }
        }

        /// Отправляет сообщение всем UDP-клиентам, кроме отправителя
        /// @param message Сообщение для отправки
        /// @param sender_socket Сокет отправителя
        void Server::broadcast_udp(const std::string &message, sockaddr_in *sender) {
            if (sender) {
                // Проверка, известен ли отправитель
                auto it = std::find_if(udp_clients_.begin(), udp_clients_.end(),
                                       [sender](const sockaddr_in& client) {
                                           return client.sin_addr.s_addr == sender->sin_addr.s_addr &&
                                                  client.sin_port == sender->sin_port;
                                       });

                // Если не найден, добавить в список клиентов
                if (it == udp_clients_.end()) {
                    udp_clients_.push_back(*sender);
                }
            }

            for (const auto& client : udp_clients_) {
                if (!sender ||
                    client.sin_addr.s_addr != sender->sin_addr.s_addr ||
                    client.sin_port != sender->sin_port) {
                    sendto(server_socket_, message.c_str(), message.length(), 0,
                           reinterpret_cast<const sockaddr*>(&client), sizeof(client));
                }
            }
        }

};
