#ifndef CURSOV_UTILS_H
#define CURSOV_UTILS_H

#include "Headers.h"

class Utils {
public:
    /**
     * @brief Печатает MAC-адрес в человекочитаемом виде.
     * @param mac Массив байт MAC-адреса.
     */
    static void printMac(const std::array<uint8_t, 6> &mac);
};

#endif //CURSOV_UTILS_H
