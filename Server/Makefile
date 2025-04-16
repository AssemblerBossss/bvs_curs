# Компилятор и флаги
CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -I. -I./utilities -I./Server/src

# Целевой исполняемый файл
TARGET = server

# Исходники
SRCS = start_server.cpp src/Server.cpp ../utilities/config.h

# Линковка с pthread
LIBS = -lpthread

# Правила
all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LIBS)

clean:
	rm -f $(TARGET)
