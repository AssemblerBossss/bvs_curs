#include "Utils.h"


void Utils::printMac(const std::array<uint8_t, 6>& mac) {
    std::stringstream stream_mac;

    for (int i = 0; i < 6; i++) {
    /**
     * - std::setw(2): задает ширину вывода в 2 символа.
     * - std::setfill('0'): если число занимает меньше 2 символов,
     *   дополняет его нулями слева.
     */
        stream_mac << std::hex << std::uppercase << std::setw(2)
        << std::setfill('0') << static_cast<int>(mac[i]);
        if (i != 5) {
            stream_mac << ":";
        }
    }
    std::cout << stream_mac.str();
}