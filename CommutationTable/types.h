#ifndef MONITORING_TYPES_H
#define MONITORING_TYPES_H

#include <cstdint>

#include <pcap.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <net/ethernet.h>

struct EthernetHeader {
    u_char dest[6];
    u_char source[6];
    uint16_t type;
};

struct IpHeader {
    uint8_t vhl;
    uint8_t tos;
    uint16_t len;
    uint16_t id;
    uint16_t offset;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t checksum;
    uint32_t src;
    uint32_t dst;
};

struct UdpHeader {
    uint16_t sport;
    uint16_t dport;
    uint16_t len;
    uint16_t checksum;
};

struct IcmpHeader {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint16_t id;
    uint16_t seq;
};

struct ArpHeader {
    uint16_t htype;         // Hardware type
    uint16_t ptype;         // Protocol type
    uint8_t hlen;           // Hardware address length
    uint8_t plen;           // Protocol address length
    uint16_t op;            // Operation code
    uint8_t sender_mac[6];  // Sender MAC address
    uint32_t sender_ip;     // Sender IP address
    uint8_t target_mac[6];  // Target MAC address
    uint32_t target_ip;     // Target IP address
};

struct TcpHeader {
    uint16_t sport;         // Source port
    uint16_t dport;         // Destination port
    uint32_t seq;           // Sequence number
    uint32_t ack_seq;       // Acknowledgement number
    uint8_t  data_offset;   // Data offset
    uint8_t  flags;         // TCP flags
    uint16_t window;        // Window size
    uint16_t checksum;      // Checksum
    uint16_t urg_ptr;       // Urgent pointer
};

#endif // MONITORING_TYPES_H