#include "CommutationTable.h"
#include "Utils.h"


int CommutationTable::row_life_time = 10;

CommutationTable::CommutationTable() {}


void CommutationTable::insert(const std::array<uint8_t, MAC_SIZE>& mac, int port) {
    std::lock_guard<std::mutex> lock(tableMutex);

    // Удаляем MAC с других портов (если есть)
    for (auto& [p, entries] : table) {
        if (p != port) {
            entries.remove_if([&mac](const TableEntry& entry) {
                return entry.get_mac() == mac;
            });
        }
    }

    // Удаляем дубликат в текущем порту (на всякий случай)
    table[port].remove_if([&mac](const TableEntry& entry) {
        return entry.get_mac() == mac;
    });

    // Добавляем новую запись
    table[port].emplace_front(mac, row_life_time);

    std::cout << "New row, port: " << port << ", mac: ";
    for (const auto& byte : mac) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << ":";
    }
    std::cout << std::dec << "\n";
}



void CommutationTable::update(const std::array<uint8_t, MAC_SIZE>& mac, int port) {
    std::lock_guard<std::mutex> lockGuard(tableMutex);
    for (TableEntry& entry: table[port]) {
        if (entry.get_mac() == mac) {
            entry.reset_life_time(row_life_time);
            return;
        }
    }
}


bool CommutationTable::contains(const std::array<uint8_t, MAC_SIZE> &mac, int port) const {
    std::lock_guard<std::mutex> lock(tableMutex);
    auto it = table.find(port);
    if (it == table.end()) { return false; }

    for (const TableEntry& entry: it->second) {
        if (entry.get_mac() == mac) { return true; }
    }
    return false;

}


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


void CommutationTable::clear_expired() {
    std::lock_guard<std::mutex> lock(tableMutex);
    for (auto& [port, entries] : table) {
        entries.remove_if([](TableEntry& entry) {
            entry.decrease_life_time();
            return entry.get_life_time() <= 0;
        });
    }
}


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

bool CommutationTable::forward_packet(const std::array<uint8_t, MAC_SIZE> &src_mac,
                                      const std::array<uint8_t, MAC_SIZE> &dst_mac, int src_port, const u_char *packet,
                                      size_t packet_size) {
    // Обучение - добавление/обновление записи для источника
    if (contains(src_mac, src_port)) {
        update(src_mac, src_port);
    } else {
        insert(src_mac, src_port);
    }

    // Проверка на широковещательный адрес (все биты = 1)
    bool is_broadcast = true;
    for (short i = 0; i < MAC_SIZE; i++) {
        if (dst_mac[i] != 0xFF) {
            is_broadcast = false;
            break;
        }
    }

    // Если это широковещательный адрес, отправляем на все порты кроме источника
    if (is_broadcast) {
        bool all_sent = true;
        for (int port = 1; port <= INTERFACES; port++) {
            if (port != src_port) {
                if (!send_to_port(port, packet, packet_size)) {
                    all_sent = false;
                }
            }
        }
        return all_sent;
    }

    // Если это не широковещательный адрес, ищем порт назначения в таблице
    int dst_port = -1;
    if (find_interface(dst_mac, dst_port)) {
        // Если порт назначения совпадает с портом источника, ничего не делаем
        if (dst_port == src_port) {
            std::cout << "Source and destination ports are the same. Packet dropped." << std::endl;
            return true; // Пакет не переслан, но обработан правильно
        }
        // Отправка пакета на порт назначения
        return send_to_port(dst_port, packet, packet_size);
    } else {
        // Если порт не найден, выполняем отправку на все порты, кроме источника
        bool all_sent = true;
        for (int port = 1; port <= INTERFACES; port++) {
            if (port != src_port) {
                if (!send_to_port(port, packet, packet_size)) {
                    all_sent = false;
                }
            }
        }

        return all_sent;
    }
}


void CommutationTable::set_life_time(int seconds) {
    row_life_time = seconds > 0 ? seconds : 10;
}