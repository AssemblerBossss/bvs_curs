#ifndef CURSOV_SERVER_H
#define CURSOV_SERVER_H
#include "../../Headers.h"
#include <cstdint>
#include "../../utilities/protocol.h"


namespace Net {

    /**
     * @class Server
     * @brief Сервер для обмена сообщениями с клиентами по TCP или UDP протоколам.
     */
    class Server {
    public:
        /**
         * @brief Конструктор сервера с указанием порта и протокола.
         * @param port Порт, на котором будет работать сервер.
         * @param protocol Протокол работы сервера (TCP по умолчанию).
         */
        Server(uint16_t port, Protocol protocol = Protocol::TCP);
        
        /**
         * @brief Деструктор сервера, освобождает ресурсы.
         */
        ~Server();

        /**
         * @brief Запускает сервер, начинает прослушивание порта.
         */
        void start();

        /**
         * @brief Останавливает сервер, закрывает соединения и освобождает ресурсы.
         */
        void stop();

    private:
        /**
         * @brief Настраивает сокет сервера в соответствии с выбранным протоколом.
         */
        void setup_socket();

        /**
         * @brief Основной цикл прослушивания для TCP-сервера.
         */
        void tcp_listen();

        /**
         * @brief Основной цикл прослушивания для UDP-сервера.
         */
        void udp_listen();

        /**
         * @brief Обрабатывает подключение TCP-клиента в отдельном потоке.
         * @param client_socket Дескриптор сокета клиента.
         * @param client_addr Структура адреса клиента.
         */
        void handle_tcp_client(int client_socket, sockaddr_in client_addr);

        /**
         * @brief Рассылает сообщение всем подключенным TCP-клиентам, кроме отправителя.
         * @param message Сообщение для отправки.
         * @param sender_socket Дескриптор сокета отправителя.
         */
        void broadcast_tcp(const std::string& message, int sender_socket);

        /**
         * @brief Рассылает сообщение всем известным UDP-клиентам, кроме отправителя.
         * @param message Сообщение для отправки.
         * @param sender Адрес отправителя (nullptr для отправки всем).
         */
        void broadcast_udp(const std::string& message, sockaddr_in* sender = nullptr);

        uint16_t port_;              ///< Порт сервера
        Protocol protocol_;          ///< Протокол работы сервера (TCP или UDP)
        int server_socket_;          ///< Дескриптор серверного сокета
        bool is_running_;            ///< Флаг работы сервера
        std::vector<int> tcp_clients_;    ///< Список дескрипторов подключенных TCP-клиентов
        std::vector<sockaddr_in> udp_clients_;  ///< Список адресов UDP-клиентов
        std::unique_ptr<std::thread> listener_thread_;  ///< Поток прослушивания соединений

    };

}

#endif //CURSOV_SERVER_H
