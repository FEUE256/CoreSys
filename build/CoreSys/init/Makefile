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
$(BUILD_DIR)/%.o: $(I_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(I_CFLAGS) -c $< -o $@

# =====================
# Compile EDK2 libraries
# =====================
$(BUILD_DIR)/lib/MdePkg/Library/%.o: $(SRC_DIR)/lib/MdePkg/Library/%.c 
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# =====================
# Object lists
# =====================
I_SRCS := $(wildcard $(I_DIR)/*.c)
ALL_OBJS := $(patsubst $(I_DIR)/%.c,$(BUILD_DIR)/%.o,$(I_SRCS)) $(LIB_OBJS)

# =====================
# Link EFI
# =====================
$(BUILD_I_EFI): $(ALL_OBJS) | $(BIN_DIR)
	@echo "ALL_OBJS = $(ALL_OBJS)"
	@echo "Linking CoreSys EFI..."
	$(CC) $(ALL_OBJS) $(I_LDFLAGS) -o $@
	@echo "Built EFI: $@"
	

# =====================
# All
# =====================
all: FORCE $(BUILD_I_EFI) gpt

FORCE:

# =====================
# Clean
# =====================
clean: 
	rm -fr "$(BUILD_DIR)/lib"
	rm -f "$(ALL_OBJS)" "$(BUILD_I_EFI)" "$(SRC_DIR/main.o)" "$(BUILD_DIR)/main.o" "$(SRC_DIR)/init/main.o"
	@echo "Cleaned build artifacts"
