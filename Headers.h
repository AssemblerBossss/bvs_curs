#ifndef CURSOV_HEADERS_H
#define CURSOV_HEADERS_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <filesystem>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <chrono>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ether.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pcap.h>
#include <iomanip>
#include <sys/stat.h>
#include <list>
#include <sys/ioctl.h>      // для ioctl
#include <net/if.h>         // для struct ifreq и IFNAMSIZ
#include <sys/types.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>


// Общие константы
const int PORT = 3425;
const std::string DEFAULT_IP = "127.0.0.1";

#define MAC_SIZE = 6

// Перечисление команд
enum class Commands {
    connect,
    list,
    upload,
    get,
    help,
    exit,
    command_error,
    argument_error
};

// Структура для передачи данных в поток
struct my_data {
    bool is_spoof;
    std::string *spoofing_ip;
    std::map<std::string, std::pair<std::string, time_t>> *commutation_table;
    std::string *current_interface;
    std::vector<std::string> *interfaces;
};


#endif //CURSOV_HEADERS_H
