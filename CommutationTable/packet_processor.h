#ifndef PACKET_PROCESSOR_H
#define PACKET_PROCESSOR_H

#include <pcap/pcap.h>
#include <cstdint>
#include <chrono>
#include <vector>
#include "types.h"
#include "commutation_table.h"

class PacketProcessor {
public:
    static void handler(uint8_t* user, const struct pcap_pkthdr* h, const uint8_t* packet);

    static void printEthernetInfo(const EthernetHeader* eth, uint32_t packet_len);
    static void printArpInfo(const ArpHeader* arp);
    static void printIpInfo(const IpHeader* ip);
    static void printIcmpInfo(const IcmpHeader* icmp, uint32_t data_size);
    static void printTcpInfo(const TcpHeader* tcp);
    static void printUdpInfo(const UdpHeader* udp);
};

#endif // PACKET_PROCESSOR_H