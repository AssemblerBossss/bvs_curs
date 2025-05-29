#include "PacketProcessor.h"

void PacketProcessor::printEthernetInfo(const struct ether_header *eth, uint32_t packet_len) {
    std::cout << "\n=== Packet (" << packet_len << " bytes) ===" << std::endl;
    std::cout << "[L2] Ethernet: src = " << utils::macToString(eth->ether_shost)
              << ", dst = " << utils::macToString(eth->ether_dhost)
              << ", type: 0x" << std::hex << ntohs(eth->ether_type) << std::dec << std::endl;
}

void PacketProcessor::printIpInfo(const struct iphdr *ip) {
    std::cout << "[L3] IP: src = " << utils::ipToString(ip->saddr)
              << ", dst = " << utils::ipToString(ip->daddr)
              << ", proto = " << static_cast<int>(ip->protocol)
              << ", ttl = " << static_cast<int>(ip->ttl)
              << ", len = " << ntohs(ip->tot_len) << " bytes" << std::endl;
}

void PacketProcessor::printIcmpInfo(const icmphdr *icmp, uint32_t data_size) {
    std::cout << "[L4] ICMP: type = " << static_cast<int>(icmp->type)
              << ", code = " << static_cast<int>(icmp->code)
              << ", size = " << data_size << " bytes" << std::endl;
}

void PacketProcessor::printTcpInfo(const struct tcphdr *tcp) {
    // Получаем длину TCP заголовка (data_offset в 32-битных словах)
    uint8_t tcp_header_len = tcp->doff * 4;

    std::cout << "[L4] TCP: sport = " << ntohs(tcp->source)
              << ", dport = " << ntohs(tcp->dest)
              << ", seq = " << ntohl(tcp->seq)
              << ", ack = " << ntohl(tcp->ack_seq)
              << ", flags: ";

    // Разбираем флаги
    if (tcp->th_flags & 0x02) std::cout << "SYN ";
    if (tcp->th_flags & 0x10) std::cout << "ACK ";
    if (tcp->th_flags & 0x01) std::cout << "FIN ";
    if (tcp->th_flags & 0x04) std::cout << "RST ";
    if (tcp->th_flags & 0x08) std::cout << "PSH ";
    if (tcp->th_flags & 0x20) std::cout << "URG ";

    std::cout << ", win = " << ntohs(tcp->window)
              << ", header len = " << static_cast<int>(tcp_header_len) << " bytes" << std::endl;
}

void PacketProcessor::printUdpInfo(const struct udphdr *udp) {
    std::cout << "[L4] UDP: sport = " << ntohs(udp->source)
              << ", dport = " << ntohs(udp->dest)
              << ", len = " << (ntohs(udp->len) - sizeof(udphdr))
              << " bytes" << std::endl;
}

void PacketProcessor::printPacketCaptureTime(const struct pcap_pkthdr *header) {
    time_t seconds = header->ts.tv_sec;
    struct tm *tm_info = localtime(&seconds);
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);

    // Выводим информацию о пакете
    printf("Capture time: %s.%06ld\n", time_str, header->ts.tv_usec);
}

void PacketProcessor::handler(uint8_t *user, const struct pcap_pkthdr *header, const uint8_t *packet) {
    const struct ether_header *eth = reinterpret_cast<const ether_header *>(packet);
    printEthernetInfo(eth, header->len);

    switch (ntohs(eth->ether_type)) {
        case ETHERTYPE_IP: {
            const struct iphdr *ip = reinterpret_cast<const iphdr *>(packet + sizeof(ether_header));
            printIpInfo(ip);

            switch (ip->protocol) {
//                case IPPROTO_ICMP: {
//                    const IcmpHeader *icmp = reinterpret_cast<const IcmpHeader *>(packet + sizeof(EthernetHeader) +
//                                                                                  sizeof(IpHeader));
//                    uint32_t data_size = ntohs(ip->len) - sizeof(IpHeader) - sizeof(IcmpHeader);
//                    printIcmpInfo(icmp, data_size);
//                    break;
//                }
                case IPPROTO_TCP: {
                    const struct tcphdr *tcp = reinterpret_cast<const tcphdr *>(
                            packet +
                            sizeof(ether_header) +
                            sizeof(iphdr)
                            );
                    printTcpInfo(tcp);
                    break;
                }
                case IPPROTO_UDP: {
                    const struct udphdr *udp = reinterpret_cast<const udphdr *>(packet + sizeof(ether_header) +
                                                                               sizeof(iphdr));
                    printUdpInfo(udp);
                    break;
                }
            }
            break;
        }
    }

    // Выводим время захвата пакета
    printPacketCaptureTime(header);
}
