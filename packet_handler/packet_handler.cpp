#include <netinet/tcp.h>
#include "PacketHandler.h"
#include "../CommutationTable/CommutationTable.h"
#include "../CommutationTable/Utils.h"

// Глобальная таблица коммутации для использования в обработчике пакетов
static CommutationTable g_commutation_table;

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

    // Преобразуем MAC-адреса в формат std::array
    std::array<uint8_t, MAC_SIZE> src_mac;
    std::array<uint8_t, MAC_SIZE> dst_mac;
    
    // Копируем MAC-адреса из заголовка Ethernet
    for (int i = 0; i < MAC_SIZE; i++) {
        src_mac[i] = etherHeader->ether_shost[i];
        dst_mac[i] = etherHeader->ether_dhost[i];
    }
         
    // Предполагаем, что порт, с которого пришел пакет, указан в args
    // Если args == nullptr, используем порт 1 как пример
    int src_port = (args != nullptr) ? *((int*)args) : 1;
    
    // Пересылка пакета с использованием таблицы коммутации
    bool forwarded = g_commutation_table.forward_packet(
        src_mac, dst_mac, src_port, packet, header->len);
    
    // Проверяем, был ли пакет IP с TCP или UDP для вывода информации
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
    char errbuf[PCAP_ERRBUF_SIZE];
    std::vector<pcap_t*> handles;
    std::vector<std::string> interfaces = {
        "enxd60d1cddb2fe",  // Замените на ваши реальные интерфейсы
        "eth0",
        "eth1",
        "eth2"
    };

    // Открываем все интерфейсы
    for (size_t i = 0; i < interfaces.size(); i++) {
        pcap_t* handle = pcap_open_live(interfaces[i].c_str(), BUFSIZ, 1, 1000, errbuf);
        if (handle == nullptr) {
            std::cerr << "Could not open device " << interfaces[i] << ": " << errbuf << std::endl;
            continue;
        }
        handles.push_back(handle);
        std::cout << "Opened interface " << interfaces[i] << " as port " << (i + 1) << std::endl;
    }

    if (handles.empty()) {
        std::cerr << "No interfaces were opened successfully" << std::endl;
        return 1;
    }

    std::cout << "Starting packet capture on " << handles.size() << " interfaces..." << std::endl;

    // Запускаем поток для периодической очистки таблицы
    std::thread cleanup_thread([]() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            g_commutation_table.clear_expired();
            g_commutation_table.print(); // Выводим состояние таблицы каждую секунду
        }
    });
    cleanup_thread.detach();

    // Запускаем захват пакетов на всех интерфейсах
    std::vector<std::thread> capture_threads;
    for (size_t i = 0; i < handles.size(); i++) {
        int port = i + 1;
        capture_threads.emplace_back([handles[i], port]() {
            pcap_loop(handles[i], 0, packet_handler, (u_char*)&port);
        });
    }

    // Ждем завершения всех потоков захвата
    for (auto& thread : capture_threads) {
        thread.join();
    }

    // Закрываем все дескрипторы
    for (auto handle : handles) {
        pcap_close(handle);
    }
    
    // Очищаем ресурсы портов перед выходом
    cleanup_port_handlers();
    
    return 0;
}
