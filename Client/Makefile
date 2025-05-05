# Компилятор и флаги
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

# Папки
SRC_DIR = src
UTIL_DIR = utilities
OBJ_DIR = build
INCLUDES = -I. -I./utilities

# Название итогового приложения
TARGET = client_app

# Все исходные файлы (можно расширить)
SRCS = main.cpp \
       $(SRC_DIR)/Client.cpp \


# Генерация object-файлов
OBJS = $(SRCS:.cpp=.o)

# Правило сборки
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all clean
