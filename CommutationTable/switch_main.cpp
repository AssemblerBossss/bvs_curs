#include "CommutationTable.h"
#include "NetworkUtils.h"
#include "config_parser.h"

// #define DEBUG

static uint16_t checksum(uint16_t *addr, int len) {
    uint32_t sum = 0;
    while (len > 1) {
        sum += *addr++;
        len -= 2;
    }
    if (len == 1) sum += *(uint8_t *) addr;
    sum = (sum >> 16) + (sum & 0xFFFF);
    return (uint16_t) (~sum);
}

bool is_target_icmp_packet(struct ip *iph, const ttl_substitution_cfg *ttl_cfg) {
    // Сравниваем IP-адреса
    bool src_match = (memcmp(&iph->ip_src, &ttl_cfg->client_ip, sizeof(struct in_addr))) == 0;
    bool dst_match = (memcmp(&iph->ip_dst, &ttl_cfg->server_ip, sizeof(struct in_addr))) == 0;

    // Либо обратное направление
    bool reverse_src_match = (memcmp(&iph->ip_src, &ttl_cfg->server_ip, sizeof(struct in_addr))) == 0;
    bool reverse_dst_match = (memcmp(&iph->ip_dst, &ttl_cfg->client_ip, sizeof(struct in_addr))) == 0;

    return (src_match && dst_match) || (reverse_src_match && reverse_dst_match);
}

uint16_t tcp_checksum(struct ip *iph, struct tcphdr *tcp) {
    uint32_t sum = 0;
    uint16_t tcp_len = ntohs(iph->ip_len) - (iph->ip_hl << 2);

    // Псевдо-заголовок (как в UDP)
    sum += (iph->ip_src.s_addr >> 16) & 0xFFFF;
    sum += iph->ip_src.s_addr & 0xFFFF;
    sum += (iph->ip_dst.s_addr >> 16) & 0xFFFF;
    sum += iph->ip_dst.s_addr & 0xFFFF;
    sum += htons(IPPROTO_TCP);
    sum += htons(tcp_len);

    // TCP заголовок и данные
    uint16_t *ptr = (uint16_t *) tcp;
    int remaining = tcp_len;

    while (remaining > 1) {
        sum += *ptr++;
        remaining -= 2;
    }

    // Если остался нечетный байт
    if (remaining == 1) {
        sum += *(uint8_t *) ptr;
    }

    // Сворачиваем переносы
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return (uint16_t) (~sum);
}

bool is_target_tcp_packet(struct ip *iph, const ttl_substitution_cfg *ttl_cfg) {
    // Сравниваем IP-адреса как в работающем UDP-коде
    bool src_match = (memcmp(&iph->ip_src, &ttl_cfg->client_ip, sizeof(struct in_addr))) == 0;
    bool dst_match = (memcmp(&iph->ip_dst, &ttl_cfg->server_ip, sizeof(struct in_addr))) == 0;

    // Либо обратное направление
    bool reverse_src_match = (memcmp(&iph->ip_src, &ttl_cfg->server_ip, sizeof(struct in_addr))) == 0;
    bool reverse_dst_match = (memcmp(&iph->ip_dst, &ttl_cfg->client_ip, sizeof(struct in_addr))) == 0;

    return (src_match && dst_match) || (reverse_src_match && reverse_dst_match);
}

void tableMaintenanceThread(CommutationTable &table, std::atomic<bool> &running) {
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        table.ageEntries();

        // Периодический вывод информации
        static int counter = 0;
        if (++counter % 5 == 0) { // Каждые 5 секунд
            table.printTable();
            table.printStats();
        }
    }
}

void processPacket(pcap_t *handle, const u_char *packet, int port,
                   CommutationTable &table, const std::vector<pcap_t *> &handles,
                   int packetLength, const ttl_substitution_cfg *ttl_cfg) {
    auto start = std::chrono::high_resolution_clock::now();
    const u_char *original_packet = packet;
    u_char *modified_packet = nullptr;

    struct ether_header *eth_header = (struct ether_header *) packet;
    table.updateStats(0);

    if (memcmp(eth_header->ether_shost, eth_header->ether_dhost, 6) == 0) {
        auto end = std::chrono::high_resolution_clock::now();
        table.updateStats(std::chrono::duration<double, std::milli>(end - start).count());
        return;
    }

    table.updateEntry(eth_header->ether_shost, port);

    // Обработка TTL подмены для ICMP
    if (ttl_cfg->is_active && ntohs(eth_header->ether_type) == ETHERTYPE_IP) {
        struct ip *iph = (struct ip *) (packet + sizeof(struct ether_header));

        if (iph->ip_p == IPPROTO_ICMP && is_target_icmp_packet(iph, ttl_cfg)) {
            // Создаем копию пакета для модификации
            modified_packet = new u_char[packetLength];
            memcpy(modified_packet, packet, packetLength);

            struct ip *mod_iph = (struct ip *) (modified_packet + sizeof(struct ether_header));

            // Устанавливаем TTL = 1 для ICMP
            mod_iph->ip_ttl = 1;

            // Пересчитываем IP checksum (для ICMP не нужно пересчитывать checksum самого ICMP)
            mod_iph->ip_sum = 0;
            mod_iph->ip_sum = checksum((uint16_t *) mod_iph, mod_iph->ip_hl << 2);

            packet = modified_packet;

#ifdef DEBUG
            printf("Modified TTL for ICMP packet: %s -> %s (TTL %d->1)\n",
                   inet_ntoa(mod_iph->ip_src), inet_ntoa(mod_iph->ip_dst),
                   iph->ip_ttl);
#endif
        }
    }

    // Остальная часть функции остается без изменений
    int dest_port = table.getPortForMac(eth_header->ether_dhost);
    if (dest_port != -1 && dest_port < (int) handles.size()) {
        pcap_sendpacket(handles[dest_port], packet, packetLength);
    } else {
        for (size_t i = 0; i < handles.size(); ++i) {
            if (i != (size_t) port) {
                pcap_sendpacket(handles[i], packet, packetLength);
            }
        }
    }

    if (modified_packet != nullptr) {
        delete[] modified_packet;
    }

    auto end = std::chrono::high_resolution_clock::now();
    table.updateStats(std::chrono::duration<double, std::milli>(end - start).count());
}

void captureThread(pcap_t *handle, int port,
                   CommutationTable &table,
                   const std::vector<pcap_t *> &handles,
                   std::atomic<bool> &running,
                   const ttl_substitution_cfg *ttl_cfg) {
    struct pcap_pkthdr header;
    const u_char *packet;

    while (running) {
        packet = pcap_next(handle, &header);
        if (packet) {
            processPacket(handle, packet, port, table, handles, header.len, ttl_cfg);
        }
    }
}

int main() {
    // Настраиваем время жизни записей
    int lifetime;
    std::cout << "Enter max entry lifetime (seconds): ";
    std::cin >> lifetime;

    // Инициализируем таблицу коммутации
    CommutationTable table(lifetime);
    std::atomic<bool> running{true};

    // Получаем список интерфейсов
    auto interfaces = utils::getAvailableInterfaces();
    if (interfaces.empty()) {
        std::cerr << "No network interfaces found!" << std::endl;
        return 1;
    }

    // Выбираем интерфейсы для работы
    utils::printInterfaces(interfaces);
    std::vector<std::string> selectedInterfaces;
    std::vector<pcap_t *> handles;

    std::cout << "Enter interface names to use (space separated): ";
    std::string line;
    std::getline(std::cin >> std::ws, line); // std::ws очищает ведущие пробелы
    std::istringstream iss(line);
    std::string iface;
    while (iss >> iface) {
        if (std::find(interfaces.begin(), interfaces.end(), iface) != interfaces.end()) {
            selectedInterfaces.push_back(iface);
        } else {
            std::cerr << "Interface " << iface << " not found!" << std::endl;
        }
    }

    // Открываем выбранные интерфейсы
    char errbuf[PCAP_ERRBUF_SIZE];
    for (const auto &iface: selectedInterfaces) {
        pcap_t *handle = pcap_open_live(iface.c_str(), BUFSIZ, 1, 1000, errbuf);
        if (handle) {
            handles.push_back(handle);
            std::cout << "Listening on interface " << iface << std::endl;
        } else {
            std::cerr << "Couldn't open interface " << iface << ": " << errbuf << std::endl;
        }
    }

    if (handles.empty()) {
        std::cerr << "No valid interfaces to listen on!" << std::endl;
        return 1;
    }

    ttl_substitution_cfg ttl_cfg = {0};
    ConfigParser config("../CommutationTable/ttl_substitution.cfg");
    if (!config.isLoaded()) {
        std::cerr << "Error loading config file!" << std::endl;
        return 1;
    }

    ttl_cfg.is_active = config.getBool("enabled", false);
    std::string client_ip = config.getString("client_ip", "");
    std::string server_ip = config.getString("server_ip", "");

    if (client_ip.empty() || server_ip.empty()) {
        std::cerr << "Client or server IP not specified in config!" << std::endl;
        return 1;
    }

    if (inet_pton(AF_INET, client_ip.c_str(), &ttl_cfg.client_ip) != 1 ||
        inet_pton(AF_INET, server_ip.c_str(), &ttl_cfg.server_ip) != 1) {
        std::cerr << "Invalid IP address format in config!" << std::endl;
        return 1;
    }

    ttl_cfg.max_ttl = config.getInt("max_ttl", 64);
    // Для отладки
#ifdef DEBUG
    std::cout << "TTL substitution config loaded: "
              << "\n  Enabled: " << ttl_cfg.is_active
              << "\n  Client IP: " << client_ip
              << "\n  Server IP: " << server_ip
              << "\n  Max TTL: " << (int)ttl_cfg.max_ttl << std::endl;
#endif

    // Только после сбора всех данных запускаем служебные потоки
    std::thread tableThread(tableMaintenanceThread, std::ref(table), std::ref(running));

    // Запускаем потоки захвата для каждого интерфейса
    std::vector<std::thread> captureThreads;
    for (size_t i = 0; i < handles.size(); ++i) {
        captureThreads.emplace_back(captureThread, handles[i], i,
                                    std::ref(table), std::ref(handles),
                                    std::ref(running), &ttl_cfg);
    }

    // Ожидаем завершения
    std::cout << "Switch is running. Press Enter to stop..." << std::endl;
    std::cin.ignore();
    std::cin.get();

    running = false;

    // Останавливаем потоки захвата
    for (auto &thread: captureThreads) {
        thread.join();
    }

    // Останавливаем поток обслуживания таблицы
    tableThread.join();

    // Закрываем интерфейсы
    for (auto handle: handles) {
        pcap_close(handle);
    }

    return 0;
}