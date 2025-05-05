#ifndef CURSOV_COMMUTATIONTABLE_H
#define CURSOV_COMMUTATIONTABLE_H

#include "Headers.h"
#include "TableEntry.h"

constexpr int INTERFACES = 4;


/**
 * @class CommutationTable
 * @brief Управляет таблицей коммутации с поддержкой нескольких портов.
 */
class CommutationTable {
public:
    CommutationTable();

    /**
     * @brief Вставить новую запись в таблицу на указанный порт.
     * @param mac MAC-адрес устройства.
     * @param port Номер порта, на который поступил пакет.
     */
    void insert(const std::array<uint8_t, MAC_SIZE>& mac, int port);

    /**
     * @brief Обновить время жизни существующей записи.
     * @param mac MAC-адрес устройства.
     * @param port Номер порта, в котором искать запись.
     */
    void update(const std::array<uint8_t, MAC_SIZE>& mac, int port);

    /**
     * @brief Проверить наличие записи на конкретном порту.
     * @param mac MAC-адрес.
     * @param port Порт для проверки.
     * @return true, если запись существует.
     */
    bool contains(const std::array<uint8_t, MAC_SIZE>& mac, int port) const;

    /**
     * @brief Найти порт по MAC-адресу.
     * @param mac MAC-адрес для поиска.
     * @param found_port Сюда будет записан найденный порт.
     * @return true, если адрес найден.
     */
    bool find_interface(const std::array<uint8_t, MAC_SIZE>& mac, int& found_port) const;

    /**
     * @brief Удалить все устаревшие записи (lifetime <= 0).
     */
    void clear_expired();


    /**
     * @brief Вывести содержимое таблицы в консоль.
     */
    void print() const;

    /**
     * @brief Установить новое значение времени жизни для новых и обновляемых записей.
     * @param seconds Новое значение времени жизни.
     */
    static void set_life_time();

private:
    std::map<int, std::list<TableEntry>> table; ///< Таблица: порт → список записей
    static int row_life_time;                          ///< Значение времени жизни по умолчанию
    mutable std::mutex tableMutex;              ///< Мьютекс для синхронизации доступа
};

#endif //CURSOV_COMMUTATIONTABLE_H
