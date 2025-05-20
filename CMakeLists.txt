cmake_minimum_required(VERSION 3.16)

project(cursov)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============ Executable: server ============
add_executable(server
        Server/start_server.cpp
        Server/src/Server.cpp
)

target_include_directories(server PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Server/src
        ${CMAKE_CURRENT_SOURCE_DIR}  # для config.h, Headers.h и PacketHandler.h
)

find_package(Threads REQUIRED)

target_link_libraries(server
        PRIVATE Threads::Threads
)

# ============ Executable: packet_sniffer ============
add_executable(packet_sniffer
        packet_handler/packet_handler.cpp
        CommutationTable/CommutationTable.cpp
)

add_executable( Commutation_table
        CommutationTable/switch_main.cpp
        CommutationTable/CommutationTable.cpp
        CommutationTable/NetworkUtils.cpp
        CommutationTable/PacketProcessor.cpp
        CommutationTable/config_parser.cpp
)

target_include_directories(Commutation_table PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/CommutationTable
        ${CMAKE_CURRENT_SOURCE_DIR}  # для PacketHandler.h, config.h, Headers.h
)

target_include_directories(packet_sniffer PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/packet_handler
        ${CMAKE_CURRENT_SOURCE_DIR}  # для PacketHandler.h, config.h, Headers.h
)

target_link_libraries(packet_sniffer
        PRIVATE pcap
)

find_library(PCAP_LIBRARY pcap)
target_link_libraries(Commutation_table PRIVATE ${PCAP_LIBRARY})
