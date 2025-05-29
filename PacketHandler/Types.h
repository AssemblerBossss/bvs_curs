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
    uint8_t vhl;          // Version (4 bits) and Header Length (4 bits).
    uint8_t tos;          // Type of Service.
    uint16_t len;         // Total size of the IP packet (header + data) in bytes.
    uint16_t id;          // Identification. Unique identifier for the packet, used to reassemble fragmented packets.
    uint16_t offset;      // Fragment Offset (13 bits) and Flags (3 bits).
    uint8_t ttl;          // Time to Live. Maximum number of hops (routers) the packet can pass.
    uint8_t protocol;     // Protocol. Indicates the protocol of the payload.
    uint16_t checksum;    // Header Checksum.
    uint32_t src;         // Source IP Address.
    uint32_t dst;         // Destination IP Address.
};

struct UdpHeader {
    uint16_t sport;         // Source port
    uint16_t dport;         // Destination port
    uint16_t len;
    uint16_t checksum;      // Check sum
};

struct IcmpHeader {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint16_t id;
    uint16_t seq;
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