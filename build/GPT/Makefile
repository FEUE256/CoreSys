.POSIX:
.PHONY: all clean

# =====================
# Config file
# =====================
CONFIG_FILE := ../../.config/gpt.cfg

ifneq ("$(wildcard $(CONFIG_FILE))","")
include $(CONFIG_FILE)
else
$(error Missing config file: $(CONFIG_FILE))
endif

all: $(TARGET)

$(TARGET): $(SOURCE)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(TARGET) *.img *.INF *.vhd
