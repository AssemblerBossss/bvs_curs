#ifndef CURSOV_PACKETHANDLER_H
#define CURSOV_PACKETHANDLER_H

#include "Headers.h"

/**
 * @brief Преобразует MAC-адрес из массива байт (u_char[6]) в строку формата "XX:XX:XX:XX:XX:XX".
 * @param mac Указатель на массив из 6 байт, представляющих MAC-адрес.
 * @return Строка с MAC-адресом в формате "XX:XX:XX:XX:XX:XX".
 */
std::string get_mac_address(const u_char *mac);

/**
 * @brief Обрабатывает захваченный пакет и выводит информацию о его заголовках.
 *
 * Эта функция анализирует Ethernet, IP, TCP и UDP заголовки пакета и выводит
 * соответствующую информацию на экран.
 *
 * @param args Пользовательские данные (не используются).
 * @param header Заголовок пакета, содержащий метаданные (время, длина и т.д.).
 * @param packet Указатель на данные пакета.
 */
void packet_handler(u_char *args, const struct pcap_pkthdr *header, const u_char* packet);


#endif //CURSOV_PACKETHANDLER_H
