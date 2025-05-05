#ifndef CURSOV_TABLEENTRY_H
#define CURSOV_TABLEENTRY_H

#include "Headers.h"

constexpr int MAC_SIZE = 6;


/**
 * @class TableEntry
 * @brief Представляет запись в таблице коммутации.
 */
class TableEntry {
public:
    /**
     * @brief Конструктор записи.
     * @param macAddr MAC-адрес.
     * @param lifetime Время жизни записи.
     */
    TableEntry(const std::array<uint8_t, MAC_SIZE>& macAddr, int lifetime);

    /**
     * @brief Получить MAC-адрес записи.
     * @return Ссылка на массив байт MAC-адреса.
     */
    const std::array<uint8_t, MAC_SIZE>& get_mac() const;

    /**
     * @brief Получить текущее значение времени жизни.
     * @return Время жизни.
     */
    int get_life_time() const;

    /**
     * @brief Сбросить время жизни.
     * @param newLifetime Новое значение времени жизни.
     */
    void reset_life_time(int newLifetime);

    /**
     * @brief Уменьшить время жизни на единицу.
     */
    void decrease_life_time();

private:
    std::array<uint8_t, MAC_SIZE> mac; ///< MAC-адрес
    int lifetime;                      ///< Время жизни в секундах
};
#endif //CURSOV_TABLEENTRY_H
