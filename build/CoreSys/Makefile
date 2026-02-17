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

$(BUILD_DIR):
	mkdir -p $@

# =====================
# Compile .c -> ELF object
# =====================
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# =====================
# Ensure BIN_DIR exists
# =====================
$(BIN_DIR):
	mkdir -p $@

# =====================
# Build EFI in BIN_DIR
# =====================
$(BUILD_EFI): $(O_FILE) | $(BIN_DIR)
	@echo "Linking EFI in BUILD_DIR: $@"
	$(CC) $(O_FILE) $(LDFLAGS) -o $@
	@echo "Built EFI application: $@"
	rm -f $(O_FILE)

# =====================
# Making IMG file
# =====================
gpt:
	@echo "Creating CoreSys image in $(MODE) mode..."
	@mkdir -p $(FINAL_IMG_DIR)
	@$(GPT_TOOL) -e $(BUILD_EFI)
	@mv $(GPT_IMG) $(FINAL_IMG)
	@rm -f $(BUILD_EFI)
	@rm -f $(GPT_FILE_TXT)
	@echo "CoreSys.img created at $(FINAL_IMG)"

# =====================
# All target
# =====================
all: FORCE $(BUILD_EFI) gpt

# =====================
# FORCE rebuild
# =====================
FORCE:

# =====================
# Clean
# =====================
clean:
	rm -f $(O_FILE) $(BUILD_EFI) $(FINAL_IMG)
	@echo "Cleaned build artifacts"
