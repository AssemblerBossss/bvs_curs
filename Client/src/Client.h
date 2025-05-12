#ifndef CURSOV_CLIENT_H
#define CURSOV_CLIENT_H

#include "../../Headers.h"
#include "../../utilities/config.h"
#include "../../utilities/protocol.h"
#include <thread>
#include <atomic>


namespace Net {

    /**
     * @class Client
     * @brief Клиент для обмена сообщениями по сети с использованием TCP или UDP протоколов.
     */
    class Client {
    private:
        std::string server_ip;              ///< IP-адрес сервера
        uint16_t server_port;               ///< Порт сервера
        uint16_t client_port;               ///< Порт клиента
        Protocol protocol;                  ///< Используемый протокол (TCP или UDP)
        bool is_connected;                  ///< Флаг состояния подключения
        
        int sock = -1;                      ///< Сокет для сетевых коммуникаций
        sockaddr_in server_addr{};          ///< Структура адреса сервера
        
        std::unique_ptr<std::thread> receiver_thread;  ///< Поток для приема сообщений
        std::atomic<bool> running{false};   ///< Флаг работы клиента

        /**
         * @brief Настраивает и инициализирует TCP соединение с сервером.
         */
        void runTCP();
        
        /**
         * @brief Настраивает и инициализирует UDP соединение с сервером.
         */
        void runUDP();
        
        /**
         * @brief Метод для прослушивания входящих сообщений в отдельном потоке.
         */
        void receive_messages();

    public:
        /**
         * @brief Конструктор клиента с инициализацией базовых полей.
         */
        Client() : is_connected(false), sock(-1) {}
        
        /**
         * @brief Деструктор клиента, освобождает ресурсы.
         */
        ~Client();

        /**
         * @brief Загружает конфигурацию из файла.
         * @param filename Путь к файлу конфигурации.
         */
        void load_config(const std::string& filename);

        /**
         * @brief Устанавливает параметры конфигурации клиента.
         * @param server_ip_ IP-адрес сервера.
         * @param server_port_ Порт сервера.
         * @param client_port_ Порт клиента.
         * @param protocol_ Используемый протокол (TCP или UDP).
         */
        void setConfigValue(
                const std::string& server_ip_,
                const std::string& server_port_,
                const std::string& client_port_,
                const std::string& protocol_
                );

        /**
         * @brief Загружает аргументы командной строки.
         * @param argc Количество аргументов командной строки.
         * @param argv Массив аргументов командной строки.
         */
        void loadArgs(int argc, char* argv[]);

        /**
         * @brief Запускает клиента в соответствии с настроенной конфигурацией.
         */
        void run();
        
        /**
         * @brief Отправляет сообщение серверу.
         * @param message Текст сообщения для отправки.
         * @return true, если отправка успешна.
         */
        bool send_message(const std::string& message);
        
        /**
         * @brief Безопасно останавливает работу клиента и освобождает ресурсы.
         */
        void stop();

    };
}; // namespace Net

#endif //CURSOV_CLIENT_H
