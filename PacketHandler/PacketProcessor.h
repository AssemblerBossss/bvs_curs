#ifndef PACKET_PROCESSOR_H
#define PACKET_PROCESSOR_H


#include "CommutationTable.h"
#include "Headers.h"

class PacketProcessor {
public:
    static void handler(uint8_t* user, const struct pcap_pkthdr* header, const uint8_t* packet);

    static void printEthernetInfo(const struct ether_header* eth, uint32_t packet_len);
    static void printIpInfo(const struct iphdr* ip);
    //static void printIcmpInfo(const IcmpHeader* icmp, uint32_t data_size);
    static void printTcpInfo(const struct tcphdr* tcp);
    static void printUdpInfo(const struct udphdr* udp);
    static void printPacketCaptureTime(const struct pcap_pkthdr* header);

};

#endif // PACKET_PROCESSOR_H