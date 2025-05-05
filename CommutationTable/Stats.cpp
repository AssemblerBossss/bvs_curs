#include "Stats.h"

void Stats::increasePacketCount() {
    ++packetCount;
}

void Stats::addProcessingTime(double time) {
    if (!times) {
        times = new double[capacity]();
    }

    times[currentIndex++] = time;
    if (time > maxTime)
        maxTime = time;

    if (currentIndex >= capacity) {
        int newCapacity = capacity * 2;
        double* newTimes = new double[newCapacity]();
        std::memcpy(newTimes, times, sizeof(double) * capacity);
        delete[] times;
        times = newTimes;
        capacity = newCapacity;
    }
}

double Stats::getAverageTime() const {
    if (!times || currentIndex == 0)
        return 0.0;

    double total = 0;
    int count = 0;

    for (int i = 0; i < currentIndex; ++i) {
        if (times[i] > 0) {
            total += times[i];
            ++count;
        }
    }

    return count > 0 ? total / count : 0.0;
}

void Stats::print() const {
    std::cout << "---- STATS ----\n";
    std::cout << "Packets processed: " << packetCount << "\n";
    std::cout << "Max processing time (ms): " << std::fixed << std::setprecision(2) << maxTime << "\n";
    std::cout << "Avg processing time (ms): " << std::fixed << std::setprecision(2) << getAverageTime() << "\n";
    std::cout << "----------------\n\n";
}
