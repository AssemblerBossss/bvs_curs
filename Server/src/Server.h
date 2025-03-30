#ifndef CURSOV_SERVER_H
#define CURSOV_SERVER_H
#include "../../Headers.h"
#include <cstdint>

namespace Net {

    class Server {
    public:
        Server(uint16_t port, const std::string& protocol = "tcp");
        ~Server();

        // Запускает сервер (начинает прослушивание порта)
        void start();

        // Останавливает сервер (закрывает соединения и освобождает ресурсы)
        void stop();

    private:
        // Настраивает сокет сервера в соответствии с выбранным протоколом
        void setup_socket();

        // Основной цикл прослушивания для TCP-сервера
        void tcp_listen();

        // Основной цикл прослушивания для UDP-сервера
        void udp_listen();

        // Обрабатывает подключение TCP-клиента (в отдельном потоке)
        void handle_tcp_client(int client_socket, sockaddr_in client_addr);

        // Рассылает сообщение всем подключенным TCP-клиентам, кроме отправителя
        void broadcast_tcp(const std::string& message, int sender_socket);

        // Рассылает сообщение всем известным UDP-клиентам, кроме отправителя
        void broadcast_udp(const std::string& message, sockaddr_in* sender = nullptr);

        // Порт сервера
        uint16_t port_;

        // Протокол работы ("tcp" или "udp")
        std::string protocol_;

        // Дескриптор серверного сокета
        int server_socket_;

        // Флаг работы сервера
        bool is_running_;

        // Список дескрипторов подключенных TCP-клиентов
        std::vector<int> tcp_clients;

        // Список адресов UDP-клиентов
        std::vector<sockaddr_in> udp_clients_;

        std::unique_ptr<std::thread> listener_thread_;

    };
}

#endif //CURSOV_SERVER_H
