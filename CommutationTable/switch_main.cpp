#include "CommutationTable.h"
#include "NetworkUtils/NetworkUtils.h"
#include "config_parser.h"


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

bool is_target_icmp_packet(struct ip *ip_header, const ttl_substitution_cfg *ttl_cfg) {
    // Сравниваем IP-адреса
    bool src_match = (memcmp(&ip_header->ip_src, &ttl_cfg->client_ip, sizeof(struct in_addr))) == 0;
    bool dst_match = (memcmp(&ip_header->ip_dst, &ttl_cfg->server_ip, sizeof(struct in_addr))) == 0;

    return (src_match && dst_match);
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
        struct ip *ip_header = (struct ip *) (packet + sizeof(struct ether_header));

        if (ip_header->ip_p == IPPROTO_ICMP && is_target_icmp_packet(ip_header, ttl_cfg)) {

            // Получаем указатель на ICMP-заголовок
            struct icmp *icmp_header = (struct icmp *)((u_char *)ip_header + (ip_header->ip_hl << 2));

            // Проверяем, является ли пакет ICMP Reply (тип 0)
            if (icmp_header->icmp_type == ICMP_ECHOREPLY) {
                // Создаем копию пакета для модификации
                modified_packet = new u_char[packetLength];
                memcpy(modified_packet, packet, packetLength);

                struct ip *mod_iph = reinterpret_cast<struct ip*>(
                        modified_packet + sizeof(struct ether_header));

                struct icmp *mod_icmp = reinterpret_cast<struct icmp*>(
                        reinterpret_cast<uint8_t*>(mod_iph) + (mod_iph->ip_hl << 2)
                );

                mod_icmp->icmp_type = ICMP_ECHO;
                mod_icmp->icmp_code = 0;

                // Пересчитываем IP checksum (для ICMP не нужно пересчитывать checksum самого ICMP)
                mod_icmp->icmp_cksum = 0;
                mod_icmp->icmp_cksum = checksum((uint16_t *)mod_icmp,
                                                ntohs(mod_iph->ip_len) - (mod_iph->ip_hl << 2));

                // Пересчитываем IP checksum
                mod_iph->ip_sum = 0;
                mod_iph->ip_sum = checksum((uint16_t *)mod_iph, mod_iph->ip_hl << 2);

                packet = modified_packet;
            }
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

    std::cout << "TTL substitution config loaded: "
              << "\n  Enabled: " << ttl_cfg.is_active
              << "\n  Client IP: " << client_ip
              << "\n  Server IP: " << server_ip;


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