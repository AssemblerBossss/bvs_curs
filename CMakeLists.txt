cmake_minimum_required(VERSION 3.28)
project(cursov)

set(CMAKE_CXX_STANDARD 20)

add_executable(cursov #main.cpp
#        PacketHandler.h
        Headers.h
        packet_handler/packet_handler.cpp
)


target_link_libraries(cursov pcap)