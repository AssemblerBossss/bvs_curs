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

/**
 * @brief Задержка выполнения на указанное количество миллисекунд
 * @param ms Время задержки в миллисекундах
 */
void sleep_ms(int ms);

/**
 * @brief Отправляет пакет данных на указанный порт коммутатора
 * @param port_number Номер порта для отправки
 * @param packet Указатель на данные пакета
 * @param packet_size Размер пакета в байтах
 * @return true в случае успешной отправки, false в случае ошибки
 */
bool send_to_port(int port_number, const u_char* packet, size_t packet_size);

/**
 * @brief Освобождает ресурсы, связанные с портами коммутатора
 * Должна быть вызвана перед завершением программы
 */
void cleanup_port_handlers();

#endif //CURSOV_UTILS_H
