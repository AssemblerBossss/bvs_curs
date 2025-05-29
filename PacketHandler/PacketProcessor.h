#ifndef PACKET_PROCESSOR_H
#define PACKET_PROCESSOR_H

#include "Headers.h"
#include "NetworkUtils.h"

/**
 * @class PacketProcessor
 * @brief Класс для обработки и анализа сетевых пакетов
 */
class PacketProcessor {
public:
    /**
     * @brief Обработчик пакетов для pcap_loop
     * @param user Пользовательские данные (не используется)
     * @param header Заголовок пакета, содержащий метаинформацию
     * @param packet Указатель на начало данных пакета
     */
    static void handler(uint8_t* user, const struct pcap_pkthdr* header, const uint8_t* packet);

    /**
     * @brief Выводит информацию о Ethernet-заголовке
     * @param eth Указатель на Ethernet-заголовок
     * @param packet_len Длина всего пакета в байтах
     */
    static void printEthernetInfo(const struct ether_header* eth, uint32_t packet_len);

    /**
     * @brief Выводит информацию о IP-заголовке
     * @param ip Указатель на IP-заголовок
     */
    static void printIpInfo(const struct iphdr* ip);

    /**
     * @brief Выводит информацию о TCP-заголовке
     * @param tcp Указатель на TCP-заголовок
     * @note Выводит порты, последовательности, флаги и длину заголовка
     */
    static void printTcpInfo(const struct tcphdr* tcp);

    /**
     * @brief Выводит информацию о UDP-заголовке
     * @param udp Указатель на UDP-заголовок
     * @note Выводит порты и длину данных
     */
    static void printUdpInfo(const struct udphdr* udp);

    /**
     * @brief Выводит время захвата пакета
     * @param header Заголовок пакета pcap с временной меткой
     */
    static void printPacketCaptureTime(const struct pcap_pkthdr* header);

    //static void printIcmpInfo(const IcmpHeader* icmp, uint32_t data_size);


};

#endif // PACKET_PROCESSOR_H