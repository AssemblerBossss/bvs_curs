# Имя выходного исполняемого файла
TARGET = packet_sniffer

# Компилятор и флаги
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++20

# Исходники
SRCS = \
    packet_handler.cpp \
    ../CommutationTable/CommutationTable.cpp \
    ../CommutationTable/Utils.cpp \
    ../CommutationTable/TableEntry.cpp

# Заголовочные файлы (добавлен путь на уровень выше)
INCLUDES = -I. -Ipacket_handler -I..

# Используемая библиотека
LIBS = -lpcap

# Цель по умолчанию
all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SRCS) -o $(TARGET) $(LIBS)

# Очистка
clean:
	rm -f $(TARGET)



# # Имя выходного исполняемого файла
# TARGET = packet_sniffer
#
# # Компилятор и флаги
# CXX = g++
# CXXFLAGS = -Wall -Wextra -std=c++20 -g
#
# # Папки
# SRC_DIR = .
# BUILD_DIR = build
#
# # Найти все .cpp файлы в проекте
# SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
# OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))
#
# # Заголовочные файлы
# INCLUDES = -I. -Ipacket_handler -ICommutationTable
#
# # Используемая библиотека
# LIBS = -lpcap
#
# # Правила
# all: $(TARGET)
#
# # Правило сборки исполняемого файла
# $(TARGET): $(OBJS)
# 	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@ $(LIBS)
#
# # Правило сборки объектных файлов
# $(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
# 	@mkdir -p $(dir $@)
# 	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
#
# # Очистка
# clean:
# 	rm -rf $(BUILD_DIR) $(TARGET)
#
# .PHONY: all clean
