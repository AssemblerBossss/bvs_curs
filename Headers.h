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
#include <netinet/udp.h>
#include <netinet/tcp.h>
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


// Общие константы
const int PORT = 3425;
const std::string DEFAULT_IP = "127.0.0.1";

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

// Прототипы функций
void pcap_func(const std::string &interface, my_data data);
//std::string get_mac_address(const u_char *mac);


#endif //CURSOV_HEADERS_H
