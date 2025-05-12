#include "Client.h"
#include <fcntl.h>  // Для fcntl

void Net::Client::loadArgs(int argc, char *argv[]) {
    std::string server_ip_, server_port_, client_port_, protocol_;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        size_t eq = arg.find('=');
        if (eq != std::string::npos) {
            std::string key = arg.substr(0, eq);
            std::string value = arg.substr(eq + 1);

            if (key == "server_ip") {
                server_ip_ = value;
            } else if (key == "server_port") {
                server_port_ = value;
            } else if (key == "client_port") {
                client_port_ = value;
            } else if (key == "protocol") {
                protocol_ = value;
            }
        }
    }
    if (!server_ip_.empty() && !server_port_.empty()
        && !client_port_.empty() && !protocol_.empty()) {
        setConfigValue(server_ip_, server_port_, client_port_, protocol_);
    } else {
        std::cerr << "Not all required arguments were provided." << std::endl;
    }
}


void Net::Client::setConfigValue(const std::string &server_ip_,
                                 const std::string &server_port_,
                                 const std::string &client_port_,
                                 const std::string &protocol_) {
    this->server_ip = server_ip_;
    this->server_port = stoi(server_port_);
    this->client_port = stoi(client_port_);
    this->protocol = parse_protocol(protocol_);
}


void Net::Client::load_config(const std::string &filename) {
    std::unordered_map<std::string, std::string> values = read_config_file(filename);
    setConfigValue(
            values.at("server_ip"),
            values.at("server_port"),
            values.at("client_port"),
            values.at("protocol")
    );
}

void Net::Client::runUDP() {
    sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0) {
        std::cerr << "Ошибка создания UDP сокета\n";
        return;
    }

    sockaddr_in client_addr{};
    this->server_addr = {}; // чтобы сохранить адрес сервера

    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(client_port);
    client_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (sockaddr*)&client_addr, sizeof(client_addr)) < 0) {
        std::cerr << "Ошибка привязки UDP-сокета\n";
        return;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr);

    is_connected = true;
    std::cout << "UDP: Готов к отправке сообщений\n";
    
    // Запуск потока для получения сообщений
    running = true;
    receiver_thread = std::make_unique<std::thread>(&Client::receive_messages, this);
}


bool Net::Client::send_message(const std::string &message) {
    if (!is_connected || sock < 0) {
        std::cerr << "Соединение не установлено!\n";
        return false;
    }

    if (this->protocol == Protocol::UDP) {
        ssize_t sent = sendto(sock, message.c_str(), message.size(), 0, (sockaddr*)&server_addr, sizeof(server_addr));

        if (sent < 0) {
            std::cerr << "Ошибка отправки UDP-сообщения\n";
            return false;
        }
        
        // Только отправляем сообщение, ответ обрабатывает receive_messages
        return true;
    }

    if (this->protocol == Protocol::TCP) {
        ssize_t sent = send(sock, message.c_str(), message.size(), 0);

        if (sent < 0) {
            std::cerr << "Ошибка отправки TCP-сообщения\n";
            return false;
        }
        
        // Только отправляем сообщение, ответ обрабатывает receive_messages
        return true;
    }
    return true;
}


void Net::Client::run() {
    if (protocol == Protocol::TCP) {
        runTCP();
    } else if (protocol == Protocol::UDP) {
        runUDP();
    } else {
        std::cerr << "Неизвестный протокол: " << to_string(protocol) << ". Используйте TCP или UDP.\n";
    }
}

void Net::Client::runTCP() {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Ошибка создания TCP сокета\n";
        return;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr);

    if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Ошибка подключения к TCP серверу\n";
        return;
    }

    this->server_addr = server_addr;
    is_connected = true;
    std::cout << "TCP: Подключено к серверу\n";
    
    // Запуск потока для получения сообщений
    running = true;
    receiver_thread = std::make_unique<std::thread>(&Client::receive_messages, this);
}

// Реализация метода receive_messages
void Net::Client::receive_messages() {
    char buffer[1024];
    
    // Пока клиент работает
    while (running) {
        if (!is_connected || sock < 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        
        if (protocol == Protocol::TCP) {
            // Установка неблокирующего режима для сокета
            int flags = fcntl(sock, F_GETFL, 0);
            fcntl(sock, F_SETFL, flags | O_NONBLOCK);
            
            // Попытка получить данные
            ssize_t received = recv(sock, buffer, sizeof(buffer) - 1, 0);
            
            if (received > 0) {
                buffer[received] = '\0';
                std::string message(buffer);
                
                // Очищаем текущую строку перед выводом нового сообщения
                std::cout << "\r                                                          \r"; // Стираем текущую строку
                std::cout.flush();
                
                // Проверяем, что это не подтверждение "OK"
                if (message == "OK") {
                    // Это подтверждение отправки, просто выводим его
                    std::cout << "[Сервер] Сообщение доставлено" << std::endl;
                } else {
                    // Это сообщение от другого клиента
                    std::cout << "[Получено] " << message << std::endl;
                }
                
                // После вывода сообщения снова показываем приглашение
                std::cout << "Введите сообщение: ";
                std::cout.flush();
            } else if (received == 0) {
                // Соединение закрыто
                std::cerr << "\n[Система] Соединение с сервером закрыто" << std::endl;
                is_connected = false;
                break;
            } else {
                // Ошибка или нет данных (EWOULDBLOCK/EAGAIN)
                if (errno != EWOULDBLOCK && errno != EAGAIN) {
                    std::cerr << "\n[Ошибка] При получении данных: " << strerror(errno) << std::endl;
                    is_connected = false;
                    break;
                }
            }
        } else if (protocol == Protocol::UDP) {
            // UDP работает аналогично, но используется recvfrom
            sockaddr_in sender_addr;
            socklen_t sender_len = sizeof(sender_addr);
            
            // Установка неблокирующего режима
            int flags = fcntl(sock, F_GETFL, 0);
            fcntl(sock, F_SETFL, flags | O_NONBLOCK);
            
            ssize_t received = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, 
                                        reinterpret_cast<sockaddr*>(&sender_addr), &sender_len);
            
            if (received > 0) {
                buffer[received] = '\0';
                std::string message(buffer);
                
                // Очищаем текущую строку перед выводом нового сообщения
                std::cout << "\r                                                          \r"; // Стираем текущую строку
                std::cout.flush();
                
                if (message == "OK") {
                    std::cout << "[Сервер] Сообщение доставлено" << std::endl;
                } else {
                    std::cout << "[Получено UDP] " << message << std::endl;
                }
                
                // После вывода сообщения снова показываем приглашение
                std::cout << "Введите сообщение: ";
                std::cout.flush();
            } else if (errno != EWOULDBLOCK && errno != EAGAIN) {
                std::cerr << "\n[Ошибка] При получении UDP данных: " << strerror(errno) << std::endl;
            }
        }
        
        // Небольшая пауза, чтобы не загружать процессор
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

// Реализация метода stop для безопасного завершения работы
void Net::Client::stop() {
    running = false;
    
    if (receiver_thread && receiver_thread->joinable()) {
        receiver_thread->join();
    }
    
    if (sock != -1) {
        close(sock);
        sock = -1;
    }
    
    is_connected = false;
}

// Деструктор
Net::Client::~Client() {
    stop();
}
