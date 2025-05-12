#include "src/Client.h"
#include <iostream>
#include <string>

#ifdef _WIN32
#include <winsock2.h>
#endif

int main(int argc, char* argv[]) {
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

    Net::Client client;

    std::cout << "Выберите способ конфигурации:\n";
    std::cout << "1 - Ввести параметры вручную\n";
    std::cout << "2 - Загрузить из конфигурационного файла\n";
    std::cout << "Ваш выбор: ";
    int choice;
    std::cin >> choice;
    std::cin.ignore(); // чтобы убрать символ новой строки после ввода числа

    if (choice == 1) {
        std::cout << "Введите параметры в формате:\n";
        std::cout << "server_ip=... server_port=... client_port=... protocol=...\n";
        std::cout << "Пример: server_ip=127.0.0.1 server_port=8080 client_port=3000 protocol=udp\n";
        std::string input_line;
        std::getline(std::cin, input_line);

        // Разбиваем строку в формате ключ=значение на argv-подобный массив
        std::vector<std::string> args_strs = { "program" };  // argv[0]
        size_t pos = 0;
        while ((pos = input_line.find(' ')) != std::string::npos) {
            args_strs.push_back(input_line.substr(0, pos));
            input_line.erase(0, pos + 1);
        }
        if (!input_line.empty()) {
            args_strs.push_back(input_line);
        }

        std::vector<char*> args;
        for (auto& s : args_strs)
            args.push_back(&s[0]);

        client.loadArgs(static_cast<int>(args.size()), args.data());

    } else if (choice == 2) {
        std::string config_path;
        std::cout << "Введите путь к конфигурационному файлу (например: tcp_client.cfg): ";
        std::getline(std::cin, config_path);
        client.load_config(config_path);
    } else {
        std::cerr << "Неверный выбор. Завершение программы.\n";
        return 1;
    }

    client.run();

    std::cout << "Введите сообщения для отправки (\"exit\" для выхода):" << std::endl;
    std::string msg;
    
    std::cout << "Введите сообщение: ";
    std::cout.flush();

    while (std::getline(std::cin, msg)) {
        if (msg == "exit") break;
        client.send_message(msg);
        std::cout << "Введите сообщение: ";
        std::cout.flush();
    }

    // Корректное завершение работы клиента
    client.stop();

#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
