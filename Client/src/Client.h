#ifndef CURSOV_CLIENT_H
#define CURSOV_CLIENT_H

#include "../../Headers.h"
#include "../../utilities/config.h"
#include "../../utilities/protocol.h"


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

        void runTCP();
        void runUDP();

    public:
        Client() = default;

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

    };
}; // namespace Net

#endif //CURSOV_CLIENT_H
