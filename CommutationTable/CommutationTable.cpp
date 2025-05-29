#include "CommutationTable.h"
#include "NetworkUtils.h"

CommutationTable::CommutationTable(int lifetime) : maxLifetimeSec_(lifetime) {}

void CommutationTable::updateEntry(const u_char* mac, int port) {
    std::string macStr = utils::macToString(mac);
    std::lock_guard<std::mutex> lock(mutex_);
    macTable_[macStr] = {port, std::chrono::steady_clock::now()};
}

int CommutationTable::getPortForMac(const u_char* mac) const {
    std::string macStr = utils::macToString(mac);
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = macTable_.find(macStr);
    return it != macTable_.end() ? it->second.port : -1;
}

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

void CommutationTable::updateStats(double duration) {
    std::lock_guard<std::mutex> lock(mutex_);
    processingTimes_.push_back(duration);
    if (duration > maxProcessingTime_) {
        maxProcessingTime_ = duration;
    }
    totalPackets_++;
}

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
