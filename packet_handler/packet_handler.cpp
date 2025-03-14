#include <netinet/tcp.h>
#include "../Headers.h"


std::string get_mac_address(uint8_t ether_host[ETH_ALEN]) {
    std::stringstream stream_mac;

    for (int i = 0; i < 6; i++) {
        /**
         * - std::setw(2): задает ширину вывода в 2 символа.
         * - std::setfill('0'): если число занимает меньше 2 символов, дополняет его нулями слева.
         */
        stream_mac << std::hex <<
                      std::uppercase <<
                      std::setw(2) <<
                      std::setfill('0') <<
                      static_cast<int>(ether_host[i]);
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

    //  ntohs (Network TO Host Short) выполняет преобразование 16-битного числа
    //  (например, порта или длины пакета) из сетевого порядка байт (big-endian) в порядок байт хоста (host byte order).

    if (ntohs(etherHeader->ether_type) == ETHERTYPE_IP) {
        struct iphdr* ipHeader = (struct iphdr*)(packet + sizeof(struct ether_header));

        std::cout << "\tIP Header: \n";
        std::cout << "\t  Source IP Address:  " << inet_ntoa(*(struct in_addr *)&ipHeader->saddr) << std::endl;
        std::cout << "\t  Destination IP Address:  " << inet_ntoa(*(struct in_addr *)&ipHeader->daddr) << std::endl;
        std::cout << "\t  Total Length:  " << ntohs(ipHeader->tot_len) << std::endl;
        std::cout << "\t  Data Length:  " << ntohs(ipHeader->tot_len) - ipHeader->ihl * 4 << std::endl;


        switch (ipHeader->protocol) {
            case IPPROTO_TCP:  {
                struct tcphdr* tcpHeader = (struct tcphdr*)(packet + sizeof(struct ether_header) + sizeof(iphdr));
                std::cout << "\t\tTCP Protocol" << std::endl;
                std::cout << "\t\t  Source Port:  " << ntohs(tcpHeader->source) << std::endl;
                std::cout << "\t\t  Destination Port:  " << ntohs(tcpHeader->dest) << std::endl;
                break;
            }

            case IPPROTO_UDP: {
                struct udphdr* updHeader = (struct udphdr*)(packet + sizeof(struct ether_header) + sizeof(iphdr));
                std::cout << "\t\tUDP Protocol" << std::endl;
                std::cout << "\t\t  Source Port:  " << ntohs(updHeader->source) << std::endl;
                std::cout << "\t\t  Destination Port:  " << ntohs(updHeader->dest) << std::endl;
                break;
            }
            default:
                break;
        }
    }
    std::cout << std::string(50, '-') << std::endl;
}


int main() {
    char errbuf[PCAP_ERRBUF_SIZE];  // Буфер для ошибок
    pcap_t *handle;  // Дескриптор для захвата пакетов

    // Открываем сетевой интерфейс для захвата пакетов
    handle = pcap_open_live("wlp0s20f3", BUFSIZ, 1, 1000, errbuf);
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
