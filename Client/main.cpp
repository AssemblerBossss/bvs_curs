#include "src/Client.h"
#include <string>

int main(int argc, char* argv[]) {
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
#endif

    Net::Client client;
    client.load_config("tcp_client.cfg");
    client.loadArgs(argc, argv);
    client.run();

    std::string msg;
    while (getline(std::cin, msg)) {
        if (msg == "exit") break;
        client.send_message(msg);
    }

#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
