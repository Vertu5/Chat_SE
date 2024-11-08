CXX = g++
CXXFLAGS = -Wall -Wextra -Wpedantic -std=gnu++17 -O2 -I include
LDFLAGS = -pthread -lrt

# Project directories
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Headers
HEADERS = $(wildcard $(INC_DIR)/*.hpp)

# Target executable
TARGET = chat

# Create necessary directories
$(shell mkdir -p $(OBJ_DIR))

# Default rule
all: $(TARGET)

# Build the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compile objects
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean
clean:
	rm -rf $(OBJ_DIR) $(TARGET)
	rm -f /tmp/*.chat
	rm -f /dev/shm/sem.chat*

# Install
install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin

# Uninstall
uninstall:
	rm -f /usr/local/bin/$(TARGET)

.PHONY: all clean install uninstall

# Debug rule
debug: CXXFLAGS += -g -DDEBUG
debug: all

