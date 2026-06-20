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
$(BUILD_DIR)/%.o: $(K_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(K_CFLAGS) -c $< -o $@
	
# =====================
# Object lists
# =====================
K_SRCS := $(wildcard $(K_DIR)/*.c) $(wildcard $(K_DIR)/drivers/intr/*.c)
ALL_OBJS := $(patsubst $(K_DIR)/%.c,$(BUILD_DIR)/%.o,$(K_SRCS)) 

# =====================
# Link EFI
# =====================
$(BUILD_K_EFI): $(ALL_OBJS) | $(BIN_DIR)
	@echo "Linking CoreSys EFI..."
	$(CC) $(ALL_OBJS) $(K_LDFLAGS) -o $@
	@echo "Built EFI: $@"
	
$(BUILD_K_EFI_NE): $(BUILD_K_EFI) $(NE_TOOL)
	@echo "Applying NE header..."
	@$(NE_TOOL) -i $< -o $@

# =====================
# All
# =====================
all: FORCE $(BUILD_K_EFI) $(BUILD_K_EFI_NE)

FORCE:

# =====================
# Clean
# =====================
clean: 
	rm -fr "$(BUILD_DIR)/lib"
	rm -f $(ALL_OBJS) "$(BUILD_K_EFI)" "$(SRC_DIR/main.o)" "$(BUILD_DIR)/main.o" "$(SRC_DIR)/init/main.o" "$(BUILD_K_EFI_NE)"
	@echo "Cleaned build artifacts"
	