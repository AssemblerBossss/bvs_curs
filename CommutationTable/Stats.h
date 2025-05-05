#ifndef CURSOV_STATS_H
#define CURSOV_STATS_H

#include "Headers.h"

/**
 * @brief Класс для сбора и отображения статистики обработки пакетов.
 */
class Stats {
public:
    /**
     * @brief Увеличивает счётчик обработанных пакетов.
     */
    void increasePacketCount();

    /**
     * @brief Добавляет время обработки очередного пакета.
     * @param time Время обработки в миллисекундах.
     */
    void addProcessingTime(double time);

    /**
     * @brief Получает среднее время обработки.
     * @return Среднее время в миллисекундах.
     */
    double getAverageTime() const;

    /**
     * @brief Печатает текущую статистику.
     */
    void print() const;

private:
    int packetCount = 0;
    int currentIndex = 0;
    int capacity = 10;
    double* times = nullptr;
    double maxTime = 0.0;
};

#endif //CURSOV_STATS_H
