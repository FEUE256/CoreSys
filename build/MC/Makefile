# =====================
# Config file
# =====================
CONFIG_FILE := ../../.config/mc.cfg

ifneq ("$(wildcard $(CONFIG_FILE))","")
include $(CONFIG_FILE)
else
$(error Missing config file: $(CONFIG_FILE))
endif

# Default rule
all: $(TARGET)

# Build rule
$(TARGET): $(SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

# Ensure target directory exists
$(BIN_DIR):
	mkdir -p "$@"

# Clean only the binary
clean:
	rm -f "$(TARGET)"

.PHONY: all clean
