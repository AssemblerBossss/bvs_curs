#include "CommutationTable.h"


/**
 * @brief Конструктор. Устанавливает стандартное время жизни для записей.
 */
CommutationTable::CommutationTable() : row_life_time(10) {}

/**
 * @brief Вставить новую запись в таблицу на указанный порт.
 * @param mac MAC-адрес устройства.
 * @param port Номер порта, на который поступил пакет.
 */
void CommutationTable::insert(const std::array<uint8_t, MAC_SIZE>& mac, int port) {
    std::lock_guard<std::mutex> lock(tableMutex);
    table[port].emplace_front(mac, row_life_time);
    std::cout << "New row, port: " << port << ", mac: ";
    for (const auto& byte : mac) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << ":";
    }
    std::cout << std::dec << "\n";
}

/**
 * @brief Обновить время жизни существующей записи.
 * @param mac MAC-адрес, по которому ищется запись.
 * @param port Порт, в котором проводится поиск.
 */
void CommutationTable::update(const std::array<uint8_t, MAC_SIZE>& mac, int port) {
    std::lock_guard<std::mutex> lockGuard(tableMutex);
    for (TableEntry& entry: table[port]) {
        if (entry.get_mac() == mac) {
            entry.reset_life_time(row_life_time);
            return;
        }
    }
}

/**
 * @brief Проверить наличие записи по MAC-адресу на конкретном порту.
 * @param mac MAC-адрес.
 * @param port Порт, в котором проводится поиск.
 * @return true, если запись найдена.
 */
bool CommutationTable::contains(const std::array<uint8_t, MAC_SIZE> &mac, int port) const {
    std::lock_guard<std::mutex> lock(tableMutex);
    auto it = table.find(port);
    if (it == table.end()) { return false; }

    for (const TableEntry& entry: it->second) {
        if (entry.get_mac() == mac) { return true; }
    }
    return false;

}

/**
 * @brief Найти порт по MAC-адресу среди всех портов.
 * @param mac MAC-адрес.
 * @param foundPort Сюда будет записан найденный порт.
 * @return true, если запись найдена.
 */
bool CommutationTable::find_interface(const std::array<uint8_t, MAC_SIZE> &mac, int &found_port) const {
    std::lock_guard<std::mutex> lock(tableMutex);
    for (const auto& [port, entries] : table) {
        for (const auto& entry : entries) {
            if (entry.get_mac() == mac) {
                found_port = port;
                return true;
            }
        }
    }
    return false;
}

/**
 * @brief Удалить все записи с истекшим временем жизни.
 */
void CommutationTable::clear_expired() {
    std::lock_guard<std::mutex> lock(tableMutex);
    for (auto& [port, entries] : table) {
        entries.remove_if([](TableEntry& entry) {
            entry.decrease_life_time();
            return entry.get_life_time() <= 0;
        });
    }
}

/**
 * @brief Вывести содержимое таблицы коммутации.
 */
void CommutationTable::print() const {
    std::lock_guard<std::mutex> lock(tableMutex);
    std::cout << "\n---- COMMUTATION TABLE ----\n";
    for (const auto& [port, entries]: table) {
        for (const TableEntry& entry: entries) {
            std::cout << "Port " << port << ": ";
            for (const auto& byte: entry.get_mac()) {
                std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << ":";
            }
            std::cout << std::dec << ", lifetime: " << entry.get_life_time() << "\n";

        }
    }
    std::cout << "---------------------------\n\n";
}

/**
 * @brief Установить новое значение времени жизни для новых/обновлённых записей.
 * @param seconds Время жизни в секундах.
 */
void CommutationTable::set_life_time() {
    row_life_time = seconds > 0 ? seconds : 10;
}