#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string>
#include <string_view>

enum class Protocol {
    TCP,
    UDP
};

/// Для преобразования из строки
inline Protocol parse_protocol(std::string_view str) {
    if (str == "tcp" || str == "TCP") return Protocol::TCP;
    if (str == "udp" || str == "UDP") return Protocol::UDP;
    throw std::invalid_argument("Invalid protocol: " + std::string(str));
}

/// Преобразует Protocol в строку
/// @param protocol Протокол
/// @return Строковое представление ("TCP" или "UDP")
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