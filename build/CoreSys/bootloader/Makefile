.PHONY: all clean FORCE gpt

# =====================
# Config file
# =====================

# THE PATHS IS IN THIS CONFIG FILE!

CONFIG_FILE := ../../.config/config.cfg

ifneq ("$(wildcard $(CONFIG_FILE))","")
include $(CONFIG_FILE)
else
$(error Missing config file: $(CONFIG_FILE))
endif

# =====================
# Ensure build dirs exist
# =====================
$(BUILD_DIR):
	mkdir -p $@

$(BIN_DIR):
	mkdir -p $@				

# CoreSys source files
$(BUILD_DIR)/%.o: $(B_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(B_CFLAGS) -c $< -o $@

$(BUILD_DIR)/boot/%.o: $(B_DIR)/boot/%.c
	@mkdir -p $(dir $@)
	$(CC) $(B_CFLAGS) -c $< -o $@

# =====================
# Compile EDK2 libraries
# =====================
$(BUILD_DIR)/lib/MdePkg/Library/%.o: $(SRC_DIR)/lib/MdePkg/Library/%.c 
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# =====================
# Object lists
# =====================
B_SRCS := $(wildcard $(B_DIR)/*.c) $(wildcard $(B_DIR)/boot/*.c)
ALL_OBJS := $(patsubst $(B_DIR)/%.c,$(BUILD_DIR)/%.o,$(B_SRCS)) $(LIB_OBJS)

# =====================
# Link EFI
# =====================
$(BUILD_EFI): $(ALL_OBJS) | $(BIN_DIR)
	@echo "ALL_OBJS = $(ALL_OBJS)"
	@echo "Linking CoreSys EFI..."
	$(CC) $(ALL_OBJS) $(B_LDFLAGS) -o $@
	@echo "Built EFI: $@"
	

# =====================
# All
# =====================
all: FORCE $(BUILD_EFI) gpt

FORCE:

# =====================
# Clean
# =====================
clean: 
	rm -fr "$(BUILD_DIR)/lib"
	rm -f "$(ALL_OBJS)" "$(BUILD_EFI)" "$(B_O_FILE)" "$(SRC_DIR/main.o)" "$(BUILD_DIR)/main.o" "$(SRC_DIR)/bootloader/boot/boot.o" "$(BUILD_DIR)/boot/boot.o"
	@echo "Cleaned build artifacts"
