.PHONY: all clean

# THE PATHS IS IN THIS CONFIG FILE!

CONFIG_FILE := ../../.config/config.cfg

ifneq ("$(wildcard $(CONFIG_FILE))","")
include $(CONFIG_FILE)
else
$(error Missing config file: $(CONFIG_FILE))
endif

all: bootloader init gpt

bootloader:
	@echo "Building CoreSys bootloader in $(MODE) mode..."
	@$(MAKE) -f $(B_MAKE) all -B
	@echo "Bootloader built successfully"

init:
	@echo "Building CoreSys init in $(MODE) mode..."
	@$(MAKE) -f $(I_MAKE) all -B
	@echo "Init built successfully"

gpt: $(BUILD_EFI)
	@echo "Creating CoreSys image in $(MODE) mode..."
	@mkdir -p $(FINAL_IMG_DIR)
	@$(GPT_TOOL) -e $(BUILD_EFI) -ae /EFI/FEUE/INIT.EFI/ $(ROOT_DIR)bin/$(MODE)/CoreSys/init.efi
	@mv $(GPT_IMG) $(FINAL_IMG)
	@rm -f $(BUILD_EFI)
	@rm -f $(GPT_FILE_TXT)
	@cp -f $(FINAL_IMG) $(DIST_LOCATION)
	@echo "CoreSys.img created at $(DIST_LOCATION)"

clean:
	@echo "Cleaning CoreSys build artifacts..."
	@$(MAKE) -f $(B_MAKE) clean
	@$(MAKE) -f $(I_MAKE) clean
	@rm -f ./SEVI.TXT
	@rm -f "$(FINAL_IMG)" "$(DIST_LOCATION)"