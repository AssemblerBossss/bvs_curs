# Имя выходного исполняемого файла
TARGET = packet_sniffer

# Компилятор и флаги
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++20

# Исходники
SRCS = packet_handler.cpp

# Заголовочные файлы
INCLUDES = -I. -Ipacket_handler

# Используемая библиотека
LIBS = -lpcap

# Цель по умолчанию
all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SRCS) -o $(TARGET) $(LIBS)

# Очистка
clean:
	rm -f $(TARGET)
