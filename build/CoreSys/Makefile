.PHONY: all clean

# THE PATHS IS IN THIS CONFIG FILE!

CONFIG_FILE := ../../.config/config.cfg

ADD_ESP_FILE ?=
ADD_ESP_PATH ?=

ifneq ($(ADD_ESP_FILE),)
ADD_ESP_DEFINED = 1
else
ADD_ESP_DEFINED = 0
endif

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
	# Lägg till EFI-filer i GPT-image utan att köra dem
	@$(GPT_TOOL) -e $(BUILD_EFI) -ae /EFI/FEUE/ $(ROOT_DIR)bin/$(MODE)/CoreSys/init.efi /EFI/UEFI/ $(ROOT_DIR)bin/Release/UEFI_Shell/SHELLX64.EFI

ifeq ($(ADD_ESP_DEFINED),1)
	@echo "Adding extra EFI file: $(ADD_ESP_FILE) -> $(ADD_ESP_PATH)"
	@$(GPT_TOOL) -e $(FINAL_IMG) -ae $(ADD_ESP_PATH) $(ADD_ESP_FILE) /EFI/FEUE/ $(ROOT_DIR)bin/$(MODE)/CoreSys/init.efi /EFI/UEFI/ $(ROOT_DIR)bin/Release/UEFI_Shell/SHELLX64.EFI
endif

	@mv $(GPT_IMG) $(FINAL_IMG)
	@rm -f $(BUILD_EFI) $(GPT_FILE_TXT)
	@rm -f $(DIST_LOCATION)
	@cp -f $(FINAL_IMG) $(DIST_LOCATION)
	@echo "CoreSys.img created at $(DIST_LOCATION)"

clean:
	@echo "Cleaning CoreSys build artifacts..."
	@$(MAKE) -f $(B_MAKE) clean
	@$(MAKE) -f $(I_MAKE) clean
	@rm -f ./SEVI.TXT
	@rm -f "$(FINAL_IMG)"