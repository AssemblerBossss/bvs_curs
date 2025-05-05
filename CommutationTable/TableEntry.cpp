#include "TableEntry.h"

TableEntry::TableEntry(const std::array<uint8_t, MAC_SIZE>& macAddr, int lifetime0)
    : mac(macAddr), lifetime(lifetime) {}


const std::array<uint8_t, MAC_SIZE>& TableEntry::get_mac() const {
    return mac;
};

int TableEntry::get_life_time() const {
    return lifetime;
}

void TableEntry::reset_life_time(int newLifetime) {
    lifetime = lifetime;
}

void TableEntry::decrease_life_time() {
    --lifetime;
}
