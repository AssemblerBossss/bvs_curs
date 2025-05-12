#ifndef CURSOV_CLIENT_H
#define CURSOV_CLIENT_H

#include "../../Headers.h"
#include "../../utilities/config.h"
#include "../../utilities/protocol.h"
#include <thread>
#include <atomic>


namespace Net {


    class Client {
    private:
        std::string server_ip;
        uint16_t server_port;
        uint16_t client_port;
        Protocol protocol;
        bool is_connected;

        int sock = -1;
        sockaddr_in server_addr{};
        
        std::unique_ptr<std::thread> receiver_thread;
        std::atomic<bool> running{false};

        void runTCP();
        void runUDP();
        
        // Метод для прослушивания входящих сообщений в отдельном потоке
        void receive_messages();

    public:
        Client() : is_connected(false), sock(-1) {}
        ~Client();

        void load_config(const std::string& filename);

        void setConfigValue(
                const std::string& server_ip_,
                const std::string& server_port_,
                const std::string& client_port_,
                const std::string& protocol_
                );

        void loadArgs(int argc, char* argv[]);

        void run();
        bool send_message(const std::string& message);
        
        // Остановка работы клиента
        void stop();

    };
}; // namespace Net

#endif //CURSOV_CLIENT_H
