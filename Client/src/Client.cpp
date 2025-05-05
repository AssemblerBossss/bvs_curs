#include "Client.h"

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
            values.at("sever_port"),
            values.at("client_port"),
            values.at("protocol")
    );
}

void Net::Client::runUDP() {
    int socket_ = socket(AF_INET, SOCK_DGRAM, 0);

    if (socket_ < 0) {
        std::cerr << "Ошибка создания UDP сокета\n";
        return;
    }

    sockaddr_in client_addr{}, server_addr{};

    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(client_port);
    client_addr.sin_addr.s_addr = INADDR_ANY;
    bind(socket_, (sockaddr*)&client_addr, sizeof(client_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr); // convert IPv4 and IPv6 addresses from text to binary form

    is_connected = true;
    std::cout << "UDP: Готов к отправке сообщений\n";
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

        char buffer[1024] = {};

        socklen_t len = sizeof(server_addr);
        ssize_t received = recvfrom(sock, buffer, sizeof(buffer), 0, reinterpret_cast<sockaddr *>(&server_addr), &len);

        if (received < 0) {
            std::cerr << "Ошибка получения UDP-ответа\n";
            return true;
        }

        std::cout << "UDP: Ответ сервера: " << buffer << std::endl;
        return true;
    }

    if (this->protocol == Protocol::TCP) {
        ssize_t sent = send(sock, message.c_str(), message.size(), 0);

        if (sent < 0) {
            std::cerr << "Ошбика отправки TCP-сообщения\n";
        }
        char buffer[1024] = {};
        ssize_t received = recv(sock, buffer, sizeof(buffer), 0);
        if (received < 0) {
            std::cerr << "Ошибка получения TCP-ответа\n";
            return false;
        }

        std::cout << "TCP: Ответ сервера: " << buffer << std::endl;
        return true;

    }
    return true;
}


void Net::Client::run() {
    if (protocol == Protocol::TCP) {
        //runTCP();
    } else if (protocol == Protocol::UDP) {
        runUDP();
    } else {
        std::cerr << "Неизвестный протокол: " << to_string(protocol) << ". Используйте TCP или UDP.\n";
    }
}