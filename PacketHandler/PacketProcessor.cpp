#include "PacketProcessor.h"
#include "Types.h"
#include "NetworkUtils.h"

void PacketProcessor::printEthernetInfo(const EthernetHeader *eth, uint32_t packet_len) {
    std::cout << "\n=== Packet (" << packet_len << " bytes) ===" << std::endl;
    std::cout << "[L2] Ethernet: src = " << utils::macToString(eth->source)
              << ", dst = " << utils::macToString(eth->dest)
              << ", type: 0x" << std::hex << ntohs(eth->type) << std::dec << std::endl;
}

void PacketProcessor::printIpInfo(const IpHeader *ip) {
    std::cout << "[L3] IP: src = " << utils::ipToString(ip->src)
              << ", dst = " << utils::ipToString(ip->dst)
              << ", proto = " << static_cast<int>(ip->protocol)
              << ", ttl = " << static_cast<int>(ip->ttl)
              << ", len = " << ntohs(ip->len) << " bytes" << std::endl;
}

void PacketProcessor::printIcmpInfo(const IcmpHeader *icmp, uint32_t data_size) {
    std::cout << "[L4] ICMP: type = " << static_cast<int>(icmp->type)
              << ", code = " << static_cast<int>(icmp->code)
              << ", size = " << data_size << " bytes" << std::endl;
}

void PacketProcessor::printTcpInfo(const TcpHeader *tcp) {
    // Получаем длину TCP заголовка (data_offset в 32-битных словах)
    uint8_t tcp_header_len = (tcp->data_offset >> 4) * 4;

    std::cout << "[L4] TCP: sport = " << ntohs(tcp->sport)
              << ", dport = " << ntohs(tcp->dport)
              << ", seq = " << ntohl(tcp->seq)
              << ", ack = " << ntohl(tcp->ack_seq)
              << ", flags: ";

    // Разбираем флаги
    if (tcp->flags & 0x02) std::cout << "SYN ";
    if (tcp->flags & 0x10) std::cout << "ACK ";
    if (tcp->flags & 0x01) std::cout << "FIN ";
    if (tcp->flags & 0x04) std::cout << "RST ";
    if (tcp->flags & 0x08) std::cout << "PSH ";
    if (tcp->flags & 0x20) std::cout << "URG ";

    std::cout << ", win = " << ntohs(tcp->window)
              << ", header len = " << static_cast<int>(tcp_header_len) << " bytes" << std::endl;
}

void PacketProcessor::printUdpInfo(const UdpHeader *udp) {
    std::cout << "[L4] UDP: sport = " << ntohs(udp->sport)
              << ", dport = " << ntohs(udp->dport)
              << ", len = " << (ntohs(udp->len) - sizeof(UdpHeader))
              << " bytes" << std::endl;
}

void PacketProcessor::handler(uint8_t *user, const struct pcap_pkthdr *h, const uint8_t *packet) {
    const EthernetHeader *eth = reinterpret_cast<const EthernetHeader *>(packet);
    printEthernetInfo(eth, h->len);

    switch (ntohs(eth->type)) {
        case ETHERTYPE_ARP: {
            const ArpHeader *arp = reinterpret_cast<const ArpHeader *>(packet + sizeof(EthernetHeader));
            printArpInfo(arp);
            break;
        }
        case ETHERTYPE_IP: {
            const IpHeader *ip = reinterpret_cast<const IpHeader *>(packet + sizeof(EthernetHeader));
            printIpInfo(ip);

            switch (ip->protocol) {
                case IPPROTO_ICMP: {
                    const IcmpHeader *icmp = reinterpret_cast<const IcmpHeader *>(packet + sizeof(EthernetHeader) +
                                                                                  sizeof(IpHeader));
                    uint32_t data_size = ntohs(ip->len) - sizeof(IpHeader) - sizeof(IcmpHeader);
                    printIcmpInfo(icmp, data_size);
                    break;
                }
                case IPPROTO_TCP: {
                    const TcpHeader *tcp = reinterpret_cast<const TcpHeader *>(packet + sizeof(EthernetHeader) +
                                                                               sizeof(IpHeader));
                    printTcpInfo(tcp);
                    break;
                }
                case IPPROTO_UDP: {
                    const UdpHeader *udp = reinterpret_cast<const UdpHeader *>(packet + sizeof(EthernetHeader) +
                                                                               sizeof(IpHeader));
                    printUdpInfo(udp);
                    break;
                }
            }
            break;
        }
    }
}