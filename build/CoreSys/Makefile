.PHONY: all clean FORCE

# =====================
# Config files
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
$(O_FILE): $(SRC_FILE) | $(BUILD_DIR)
	@echo "Compiling $< → $@"
	$(COMPILER) -target x86_64-efi $(CFLAGS) -I$(SRC_DIR)/include -c $< -o $@

# =====================
# Build EFI in BUILD_DIR
# =====================
$(BUILD_EFI): $(O_FILE) | $(BUILD_DIR)
	@echo "Linking EFI in BUILD_DIR: $@"
	x86_64-w64-mingw32-objcopy -O pei-x86-64 $(O_FILE) $@
	@echo "Built EFI application: $@"
	rm -f $(O_FILE)

# =====================
# All target
# =====================
all: FORCE $(BUILD_EFI)

# =====================
# FORCE rebuild
# =====================
FORCE:

# =====================
# Clean
# =====================
clean:
	rm -f $(O_FILE) $(BUILD_EFI)
	@echo "Cleaned build artifacts"
