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
$(BUILD_DIR)/%.o: $(C_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(C_CFLAGS) -c $< -o $@

# =====================
# Compile EDK2 libraries
# =====================
$(BUILD_DIR)/lib/MdePkg/Library/%.o: $(SRC_DIR)/lib/MdePkg/Library/%.c 
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# =====================
# Object lists
# =====================
C_SRCS := $(wildcard $(C_DIR)/*.c)
ALL_OBJS := $(patsubst $(C_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SRCS)) $(LIB_OBJS)

# =====================
# Link EFI
# =====================
$(BUILD_C_EFI): $(ALL_OBJS) | $(BIN_DIR)
	@echo "ALL_OBJS = $(ALL_OBJS)"
	@echo "Linking CoreSys EFI..."
	$(CC) $(ALL_OBJS) $(C_LDFLAGS) -o $@
	@echo "Built EFI: $@"
	
$(BUILD_C_EFI_NE): $(BUILD_C_EFI) $(NE_TOOL)
	@echo "Applying NE header..."
	@$(NE_TOOL) -i $< -o $@

# =====================
# All
# =====================
all: FORCE $(BUILD_C_EFI) $(BUILD_C_EFI_NE)

FORCE:

# =====================
# Clean
# =====================
clean: 
	rm -fr "$(BUILD_DIR)/lib"
	rm -f "$(ALL_OBJS)" "$(BUILD_C_EFI)" "$(SRC_DIR/main.o)" "$(BUILD_DIR)/main.o" "$(SRC_DIR)/c/main.o" "$(BUILD_C_EFI_NE)"
	@echo "Cleaned build artifacts"
