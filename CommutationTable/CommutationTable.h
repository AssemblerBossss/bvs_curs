#ifndef COMMUTATION_TABLE_H
#define COMMUTATION_TABLE_H

#include "../Headers.h"

/**
 * @class CommutationTable
 * @brief Класс для управления таблицей коммутации сетевых устройств
 *
 * Обеспечивает хранение и обновление MAC-адресов, портов и статистики обработки пакетов
 */
class CommutationTable {
public:
    /**
     * @struct TableEntry
     * @brief Структура для хранения информации о записи в таблице коммутации
     */
    struct TableEntry {
        int port; ///< Номер порта, связанного с MAC-адресом
        std::chrono::steady_clock::time_point lastSeen; ///< Время последней активности
    };

    /**
     * @brief Конструктор таблицы коммутации
     * @param lifetime Время жизни записей в секундах
     */
    CommutationTable(int lifetime);

    /**
     * @brief Обновляет или добавляет запись в таблицу
     * @param mac Указатель на MAC-адрес
     * @param port Номер порта для обновления
     */
    void updateEntry(const u_char* mac, int port);

    /**
     * @brief Возвращает порт для указанного MAC-адреса
     * @param mac Указатель на MAC-адрес
     * @return Номер порта или -1 если не найден
     */
    int getPortForMac(const u_char* mac) const;

    /**
     * @brief Удаляет устаревшие записи из таблицы
     */
    void ageEntries();

    /**
     * @brief Обновляет статистику обработки пакетов
     * @param duration Время обработки пакета в миллисекундах
     */
    void updateStats(double duration);

    /**
     * @brief Выводит текущее состояние таблицы
     */
    void printTable() const;

    /**
     * @brief Выводит статистику обработки пакетов
     */
    void printStats() const;

private:
    std::unordered_map<std::string, TableEntry> macTable_; ///< Хэш-таблица MAC-адресов
    mutable std::mutex mutex_;                             ///< Мьютекс для потокобезопасного доступа
    int maxLifetimeSec_;                                   ///< Максимальное время жизни записи в секундах

    // Статистические данные
    std::vector<double> processingTimes_;     ///< Времена обработки пакетов
    double maxProcessingTime_ = 0;            ///< Максимальное время обработки
    std::atomic<uint64_t> totalPackets_{0};   ///< Счетчик обработанных пакетов
};

#endif