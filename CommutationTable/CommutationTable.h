#ifndef COMMUTATION_TABLE_H
#define COMMUTATION_TABLE_H

#include "../Headers.h"

class CommutationTable {
public:
    struct TableEntry {
        int port;
        std::chrono::steady_clock::time_point lastSeen;
    };

    CommutationTable(int lifetime);
    void updateEntry(const u_char* mac, int port);
    int getPortForMac(const u_char* mac) const;
    void ageEntries();
    void updateStats(double duration);
    void printTable() const;
    void printStats() const;

private:
    std::unordered_map<std::string, TableEntry> macTable_;
    mutable std::mutex mutex_;
    int maxLifetimeSec_;

    // Statistics
    std::vector<double> processingTimes_;
    double maxProcessingTime_ = 0;
    std::atomic<uint64_t> totalPackets_{0};
};

#endif