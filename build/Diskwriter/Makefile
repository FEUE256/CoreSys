# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS =

# Paths
SRC_DIR = ../../src/diskwriter
BUILD_DIR = .
BIN_DIR = ../../bin/Release/Diskwriter

# Sources and objects
SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/device.c $(SRC_DIR)/writer.c $(SRC_DIR)/verify.c
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

# Target executable
TARGET = $(BIN_DIR)/diskwriter.run

all: $(TARGET)

# Link object files into final executable
$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "[Make] Build complete, removing object files..."
	rm -f $(OBJS)

# Compile each source into an object file
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Create output directory if it doesn't exist
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Clean target (tar bort allt)
clean:
	rm -f $(BUILD_DIR)/*.o
	rm -f $(TARGET)
