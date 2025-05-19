#include "Utils.h"

// Структура для хранения pcap_t* дескрипторов для каждого порта
static std::map<int, pcap_t*> port_handlers;

void sleep_ms(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// Инициализирует обработчик для указанного порта, если он ещё не инициализирован
bool init_port_handler(int port_number) {
    // Проверяем, был ли уже инициализирован обработчик для этого порта
    if (port_handlers.find(port_number) != port_handlers.end()) {
        return true; // Обработчик уже инициализирован
    }

    char errbuf[PCAP_ERRBUF_SIZE];
    std::string interface_name;

    // Определяем имя интерфейса на основе номера порта
    switch (port_number) {
        case 1:
            interface_name = "enxd60d1cddb2fe"; // USB Ethernet adapter
            break;
        case 2:
            interface_name = "eth0";
            break;
        case 3:
            interface_name = "eth1";
            break;
        case 4:
            interface_name = "eth2";
            break;
        default:
            std::cerr << "Invalid port number: " << port_number << std::endl;
            return false;
    }

    // Открываем интерфейс для инжекции пакетов
    pcap_t* handle = pcap_open_live(interface_name.c_str(), BUFSIZ, 1, 1000, errbuf);
    if (handle == nullptr) {
        std::cerr << "Could not open device " << interface_name << ": " << errbuf << std::endl;
        return false;
    }

    // Сохраняем дескриптор в карте
    port_handlers[port_number] = handle;
    std::cout << "Initialized handler for port " << port_number << " (interface " << interface_name << ")" << std::endl;
    return true;
}

bool send_to_port(int port_number, const u_char* packet, size_t packet_size) {
    // Проверяем валидность порта
    if (port_number < 1 || port_number > INTERFACES) {
        std::cerr << "Invalid port number: " << port_number << std::endl;
        return false;
    }

    // Инициализируем обработчик порта, если он ещё не инициализирован
    if (!init_port_handler(port_number)) {
        return false;
    }

    // Получаем дескриптор для порта
    pcap_t* handle = port_handlers[port_number];

    // Отправляем пакет
    if (pcap_sendpacket(handle, packet, static_cast<int>(packet_size)) != 0) {
        std::cerr << "Error sending packet on port " << port_number << ": " << pcap_geterr(handle) << std::endl;
        return false;
    }

    std::cout << "Packet sent successfully to port " << port_number << std::endl;
    return true;
}

// Освобождаем ресурсы при выходе из программы
void cleanup_port_handlers() {
    for (auto& [port, handle] : port_handlers) {
        if (handle != nullptr) {
            pcap_close(handle);
        }
    }
    port_handlers.clear();
}

void Utils::printMac(const std::array<uint8_t, 6>& mac) {
    std::stringstream stream_mac;

    for (int i = 0; i < 6; i++) {
    /**
     * - std::setw(2): задает ширину вывода в 2 символа.
     * - std::setfill('0'): если число занимает меньше 2 символов,
     *   дополняет его нулями слева.
     */
        stream_mac << std::hex << std::uppercase << std::setw(2)
        << std::setfill('0') << static_cast<int>(mac[i]);
        if (i != 5) {
            stream_mac << ":";
        }
    }
    std::cout << stream_mac.str();
}