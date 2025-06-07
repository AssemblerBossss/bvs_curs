#include "CommutationTable.h"
#include "NetworkUtils/NetworkUtils.h"

/**
 * @brief Конструктор таблицы коммутации
 * @param lifetime Время жизни записей в секундах
 */
CommutationTable::CommutationTable(int lifetime) : maxLifetimeSec_(lifetime) {}

/**
 * @brief Обновляет или добавляет запись в таблицу
 * @param mac Указатель на MAC-адрес
 * @param port Номер порта для обновления
 */
void CommutationTable::updateEntry(const u_char* mac, int port) {
    std::string macStr = utils::macToString(mac);
    std::lock_guard<std::mutex> lock(mutex_);
    macTable_[macStr] = {port, std::chrono::steady_clock::now()};
}

/**
 * @brief Возвращает порт для указанного MAC-адреса
 * @param mac Указатель на MAC-адрес
 * @return Номер порта или -1 если не найден
 */
int CommutationTable::getPortForMac(const u_char* mac) const {
    std::string macStr = utils::macToString(mac);
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = macTable_.find(macStr);
    return it != macTable_.end() ? it->second.port : -1;
}

/**
 * @brief Удаляет устаревшие записи из таблицы
 */
void CommutationTable::ageEntries() {
    auto now = std::chrono::steady_clock::now();
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto it = macTable_.begin(); it != macTable_.end();) {
        auto age = std::chrono::duration_cast<std::chrono::seconds>(now - it->second.lastSeen).count();
        if (age >= maxLifetimeSec_) {
            it = macTable_.erase(it);
        } else {
            ++it;
        }
    }
}

/**
 * @brief Обновляет статистику обработки пакетов
 * @param duration Время обработки пакета в миллисекундах
 */
void CommutationTable::updateStats(double duration) {
    std::lock_guard<std::mutex> lock(mutex_);
    processingTimes_.push_back(duration);
    if (duration > maxProcessingTime_) {
        maxProcessingTime_ = duration;
    }
    totalPackets_++;
}

/**
 * @brief Выводит текущее состояние таблицы
 */
void CommutationTable::printTable() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::cout << "\n=== MAC Table (" << macTable_.size() << " entries) ===" << std::endl;
    std::cout << std::left << std::setw(20) << "MAC Address"
              << std::setw(10) << "Port"
              << "Age (sec)" << std::endl;

    auto now = std::chrono::steady_clock::now();
    for (const auto& entry : macTable_) {
        auto age = std::chrono::duration_cast<std::chrono::seconds>(now - entry.second.lastSeen).count();
        std::cout << std::left << std::setw(20) << entry.first
                  << std::setw(10) << entry.second.port
                  << age << std::endl;
    }
    std::cout << "=============================" << std::endl;
}


/**
 * @brief Выводит статистику обработки пакетов
 */
void CommutationTable::printStats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    double avg = 0;
    if (!processingTimes_.empty()) {
        for (auto t : processingTimes_) {
            avg += t;
        }
        avg /= processingTimes_.size();
    }

    std::cout << "\n=== Statistics ==="
              << "\nTotal packets: " << totalPackets_
              << "\nAverage process time: " << avg << " ms"
              << "\nMax process time: " << maxProcessingTime_ << " ms"
              << "\nMAC table size: " << macTable_.size() << " entries"
              << "\n==================" << std::endl;
}
