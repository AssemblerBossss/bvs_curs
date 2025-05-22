cmake_minimum_required(VERSION 3.16)
project(cursov LANGUAGES CXX)

# Общие настройки проекта
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Поиск зависимостей
find_package(Threads REQUIRED)
find_library(PCAP_LIBRARY pcap)
if(NOT PCAP_LIBRARY)
    message(FATAL_ERROR "libpcap not found! Install with: sudo apt install libpcap-dev")
endif()

# Общие include директории
set(COMMON_INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/src
        ${CMAKE_CURRENT_SOURCE_DIR}/CommutationTable
        ${CMAKE_CURRENT_SOURCE_DIR}/PacketHandler
        )

# ============ Сервер ============
add_executable(server
        Server/start_server.cpp
        Server/src/Server.cpp
        )

target_include_directories(server PRIVATE ${COMMON_INCLUDES})
target_link_libraries(server PRIVATE Threads::Threads)

# ============ Packet Sniffer ============
add_executable(packet_sniffer
        PacketHandler/packet_handler.cpp
        CommutationTable/CommutationTable.cpp
        )

target_include_directories(packet_sniffer PRIVATE ${COMMON_INCLUDES})
target_link_libraries(packet_sniffer PRIVATE ${PCAP_LIBRARY})

# ============ Commutation Table ============
add_executable(Commutation_table
        CommutationTable/switch_main.cpp
        CommutationTable/CommutationTable.cpp
        CommutationTable/NetworkUtils.cpp
        PacketHandler/PacketProcessor.cpp
        CommutationTable/config_parser.cpp
        )

target_include_directories(Commutation_table PRIVATE ${COMMON_INCLUDES})
target_link_libraries(Commutation_table PRIVATE ${PCAP_LIBRARY})