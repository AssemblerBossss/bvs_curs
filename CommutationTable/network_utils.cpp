#include "../../include/utils/network_utils.h"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <ifaddrs.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>

// Для macOS
#ifdef __APPLE__
#include <net/if_dl.h>
#endif

// Для Linux
#ifdef __linux__
#include <linux/if_packet.h>
#endif

namespace utils {

std::vector<std::string> getAvailableInterfaces() {
    std::vector<std::string> interfaces;
    struct ifaddrs *ifaddr, *ifa;

    if (getifaddrs(&ifaddr) == -1) {
        std::cerr << "Failed to get network interfaces" << std::endl;
        return interfaces;
    }

    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) continue;

        // Проверяем тип адреса в зависимости от ОС
#ifdef __linux__
        if (ifa->ifa_addr->sa_family == AF_PACKET) {
#elif defined(__APPLE__)
        if (ifa->ifa_addr->sa_family == AF_LINK) {
#endif
            // Проверяем, не добавлен ли интерфейс уже
            bool exists = false;
            for (const auto& name : interfaces) {
                if (name == ifa->ifa_name) {
                    exists = true;
                    break;
                }
            }
            if (!exists) {
                interfaces.emplace_back(ifa->ifa_name);
            }
        }
    }

    freeifaddrs(ifaddr);
    return interfaces;
}

void printInterfaces(const std::vector<std::string>& interfaces) {
    std::cout << "Available network interfaces:" << std::endl;
    for (const auto& iface : interfaces) {
        std::cout << "- " << iface << std::endl;
    }
}

void printMacAddress(const uint8_t* mac) {
    printf("%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

std::string macToString(const uint8_t* mac) {
    char buf[18];
    snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return std::string(buf);
}

std::string ipToString(uint32_t ip) {
    struct in_addr addr;
    addr.s_addr = ip;
    char ipStr[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &addr, ipStr, INET_ADDRSTRLEN) == nullptr) {
        return "0.0.0.0";
    }
    return std::string(ipStr);
}



} // namespace utils