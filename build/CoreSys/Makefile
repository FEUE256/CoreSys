.PHONY: all clean FORCE gpt

# =====================
# Config file
# =====================
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

# =====================
# Compile CoreSys source
# =====================
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# =====================
# Compile EDK2 libraries
# =====================
$(BUILD_DIR)/lib/MdePkg/Library/%.o: $(SRC_DIR)/lib/MdePkg/Library/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# =====================
# Object lists
# =====================
LIB_OBJS := $(patsubst $(SRC_DIR)/lib/MdePkg/Library/%.c,$(BUILD_DIR)/lib/MdePkg/Library/%.o,$(LIB_SRCS))

ALL_OBJS := $(O_FILE) $(LIB_OBJS)

# =====================
# Link EFI
# =====================
$(BUILD_EFI): $(ALL_OBJS) | $(BIN_DIR)
	@echo "Linking CoreSys EFI..."
	$(CC) $(ALL_OBJS) $(LDFLAGS) -o $@
	@echo "Built EFI: $@"

# =====================
# Create GPT image
# =====================
gpt: $(BUILD_EFI)
	@echo "Creating CoreSys image in $(MODE) mode..."
	@mkdir -p $(FINAL_IMG_DIR)
	@$(GPT_TOOL) -e $(BUILD_EFI)
	@mv $(GPT_IMG) $(FINAL_IMG)
	@rm -f $(BUILD_EFI)
	@rm -f $(GPT_FILE_TXT)
	@echo "CoreSys.img created at $(FINAL_IMG)"

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
	rm -f "$(O_FILE)" "$(BUILD_EFI)" "$(FINAL_IMG)"  "$(BUILD_DIR)/efi.o"
	@echo "Cleaned build artifacts"
