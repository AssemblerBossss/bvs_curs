#include <netinet/tcp.h>
#include "PacketHandler.h"

std::string get_mac_address(uint8_t ether_host[ETH_ALEN]) {
    std::stringstream stream_mac;

    for (int i = 0; i < 6; i++) {
        /**
         * - std::setw(2): задает ширину вывода в 2 символа.
         * - std::setfill('0'): если число занимает меньше 2 символов,
         *   дополняет его нулями слева.
         */
        stream_mac << std::hex << std::uppercase << std::setw(2)
                   << std::setfill('0') << static_cast<int>(ether_host[i]);
        if (i != 5) {
            stream_mac << ":";
        }
    }
    return stream_mac.str();
}

void packet_handler(u_char *args, const struct pcap_pkthdr *header,
                    const u_char* packet) {
    struct ether_header* etherHeader = (struct ether_header*)packet;

    if (ntohs(etherHeader->ether_type) != ETHERTYPE_IP) {
        return; // Не IP — ничего не выводим
    }

    struct iphdr* ipHeader = (struct iphdr*)(packet + sizeof(struct ether_header));
    int protocol = ipHeader->protocol;

    // Проверка: если не TCP и не UDP — игнорируем
    if (protocol != IPPROTO_TCP && protocol != IPPROTO_UDP) {
        return;
    }

    // Теперь можно выводить всё, так как это IP + TCP или UDP
    std::cout << "Ethernet Header: \n";
    std::cout << "  Source Address:  "
              << get_mac_address(etherHeader->ether_shost) << std::endl;
    std::cout << "  Destination Address:  "
              << get_mac_address(etherHeader->ether_dhost) << std::endl;

    std::cout << "\tIP Header: \n";
    std::cout << "\t  Source IP Address:  "
              << inet_ntoa(*(struct in_addr *)&ipHeader->saddr) << std::endl;
    std::cout << "\t  Destination IP Address:  "
              << inet_ntoa(*(struct in_addr *)&ipHeader->daddr) << std::endl;
    std::cout << "\t  Total Length:  "
              << ntohs(ipHeader->tot_len) << std::endl;
    std::cout << "\t  Data Length:  "
              << ntohs(ipHeader->tot_len) - ipHeader->ihl * 4 << std::endl;

    switch (protocol) {
        case IPPROTO_TCP:  {
            struct tcphdr* tcpHeader = (struct tcphdr*)(packet +
                                                        sizeof(struct ether_header) + sizeof(iphdr));
            std::cout << "\t\tTCP Protocol" << std::endl;
            std::cout << "\t\t  Source Port:  "
                      << ntohs(tcpHeader->source) << std::endl;
            std::cout << "\t\t  Destination Port:  "
                      << ntohs(tcpHeader->dest) << std::endl;
            break;
        }

        case IPPROTO_UDP: {
            auto* udpHeader = (struct udphdr*)(packet +
                                               sizeof(struct ether_header) + sizeof(iphdr));
            std::cout << "\t\tUDP Protocol" << std::endl;
            std::cout << "\t\t  Source Port:  "
                      << ntohs(udpHeader->source) << std::endl;
            std::cout << "\t\t  Destination Port:  "
                      << ntohs(udpHeader->dest) << std::endl;
            break;
        }
    }

    std::cout << std::string(50, '-') << std::endl;
}

int main() {
    char errbuf[PCAP_ERRBUF_SIZE];  // Буфер для ошибок
    pcap_t *handle;  // Дескриптор для захвата пакетов

    // Открываем сетевой интерфейс для захвата пакетов
    handle = pcap_open_live("enxd60d1cddb2fe", BUFSIZ, 1, 1000, errbuf);
    if (handle == nullptr) {
        std::cerr << "Could not open device: " << errbuf << std::endl;
        return 1;
    }

    std::cout << "Starting packet capture..." << std::endl;
    pcap_loop(handle, 0, packet_handler, nullptr);

    // Закрываем дескриптор
    pcap_close(handle);
    return 0;
}
