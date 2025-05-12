#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string>
#include <string_view>

/**
 * @enum Protocol
 * @brief Перечисление поддерживаемых сетевых протоколов.
 */
enum class Protocol {
    TCP,  ///< Протокол TCP 
    UDP   ///< Протокол UDP 
};

/**
 * @brief Преобразует строковое представление протокола в соответствующий тип Protocol.
 * @param str Строка "tcp", "TCP", "udp" или "UDP".
 * @return Соответствующее значение перечисления Protocol.
 * @throws std::invalid_argument Если переданная строка не соответствует ни одному протоколу.
 */
inline Protocol parse_protocol(std::string_view str) {
    if (str == "tcp" || str == "TCP") return Protocol::TCP;
    if (str == "udp" || str == "UDP") return Protocol::UDP;
    throw std::invalid_argument("Invalid protocol: " + std::string(str));
}

/**
 * @brief Преобразует значение типа Protocol в строковое представление.
 * @param protocol Значение перечисления Protocol.
 * @return Строковое представление протокола ("TCP" или "UDP").
 */
inline std::string to_string(Protocol protocol) {
    switch (protocol) {
        case Protocol::TCP:
            return "TCP";
        case Protocol::UDP:
            return "UDP";
    }
    return "UNKNOWN";
}

#endif // PROTOCOL_H