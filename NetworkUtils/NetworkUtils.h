#ifndef NETWORK_UTILS_H
#define NETWORK_UTILS_H

#include "Headers.h"


namespace utils {

    std::vector<std::string> getAvailableInterfaces();
    void printInterfaces(const std::vector<std::string>& interfaces);
    std::string ipToString(uint32_t ip);
    std::string macToString(const uint8_t* mac);

} // namespace utils

#endif // NETWORK_UTILS_H