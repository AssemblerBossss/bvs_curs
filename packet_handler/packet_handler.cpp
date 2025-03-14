#include "../Headers.h"


std::string get_mac_address(const u_char *mac) {
    std::stringstream stream_mac;

    for (int i = 0; i < 6; i++) {
        /**
         * - std::setw(2): задает ширину вывода в 2 символа.
         * - std::setfill('0'): если число занимает меньше 2 символов, дополняет его нулями слева.
         */
        stream_mac << std::hex << std::setw(2) << std::setfill('0') << (int) mac[i];
        if (i != 5) {
            stream_mac << ":";
        }
    }
    return stream_mac.str();
}


void packet_handler(u_char *args, const struct pcap_pkthdr *header, const u_char* packet) {

    struct ether_header* etherHeader = (struct ether_header*)packet;

    std::cout << "Ethernet Header: \n";
    std::cout << "  Source Address:  " << get_mac_address(etherHeader->ether_shost) << std::endl;
    std::cout << "  Destination Address:  " << get_mac_address(etherHeader->ether_dhost) << std::endl;

    std::cout << std::string(50, '-') << std::endl; // Разделитель между пакетами

}


int main() {
    char errbuf[PCAP_ERRBUF_SIZE]; // Буфер для ошибок
    pcap_t *handle; // Дескриптор для захвата пакетов

    // Открываем сетевой интерфейс для захвата пакетов
    handle = pcap_open_live("wlp0s20f3", BUFSIZ, 1, 1000, errbuf);
    if (handle == nullptr) {
        std::cerr << "Could not open device: " << errbuf << std::endl;
        return 1;
    }

    // Запускаем захват пакетов
    std::cout << "Starting packet capture..." << std::endl;
    pcap_loop(handle, 0, packet_handler, nullptr);

    // Закрываем дескриптор
    pcap_close(handle);
    return 0;
}
