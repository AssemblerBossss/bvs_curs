#include "src/Server.h"
#include "../utilities/config.h"


int main() {
    try {
        auto config = read_config_file("../config");

        // Получаем порт и протокол из конфигурации
        uint16_t port = std::stoi(config.at("port"));
        Protocol protocol = parse_protocol(config.at("protocol"));

        Net::Server server(port, protocol);
        server.start();

        std::cout << "Введите 'stop' или 'quit' для завершения работы сервера." << std::endl;
        std::string command;

        while (true) {
            std::getline(std::cin, command);
            std::transform(command.begin(), command.end(), command.begin(), ::tolower);

            if (command == "stop" || command == "quit") {
                break;
            }

            std::cout << "Неизвестная команда: " << command << ". Используйте 'stop' или 'quit'." << std::endl;
        }

        server.stop();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}