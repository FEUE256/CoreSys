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

all: bootloader init kernel c gpt 

bootloader:
	@echo "Building CoreSys bootloader in $(MODE) mode..."
	@$(MAKE) -f $(B_MAKE) all -B
	@echo "Bootloader built successfully"

init:
	@echo "Building CoreSys init in $(MODE) mode..."
	@$(MAKE) -f $(I_MAKE) all -B
	@echo "Init built successfully"


kernel:
	@echo "Building CoreSys kernel in $(MODE) mode..."
	@$(MAKE) -f $(K_MAKE) all -B
	@echo "Kernel built successfully"

c:
	@echo "Building CoreSys c in $(MODE) mode..."
	@$(MAKE) -f $(C_MAKE) all -B
	@echo "C built successfully"


gpt: $(BUILD_EFI)
	@echo "Creating CoreSys image in $(MODE) mode..."
	@mkdir -p $(FINAL_IMG_DIR)
	@rm -f $(DIST_LOCATION)
	@rm -f ../../dist/.__smb008C
	@$(GPT_TOOL) -e $(BUILD_EFI) -ae /EFI/FEUE/ $(ROOT_DIR)bin/$(MODE)/CoreSys/init.re /EFI/UEFI/ $(ROOT_DIR)bin/Release/UEFI_Shell/SHELLX64.RE /EFI/FEUE/ $(ROOT_DIR)bin/$(MODE)/CoreSys/kernel.re /PU/ $(ROOT_DIR)bin/$(MODE)/CoreSys/c.re /EFI/GRUB/ $(ROOT_DIR).config/grub.cfg /EFI/LINUX/ $(ROOT_DIR)bin/Release/linux/bzImage /EFI/LINUX/ $(ROOT_DIR)build/linux/gz.gz /EFI/GRUB/ $(ROOT_DIR)bin/Release/GRUB/grubx64.re

ifeq ($(ADD_ESP_DEFINED),1)
	@echo "Adding extra EFI file: $(ADD_ESP_FILE) -> $(ADD_ESP_PATH)"
	@$(GPT_TOOL) -e $(FINAL_IMG) -ae $(ADD_ESP_PATH) $(ADD_ESP_FILE) /EFI/FEUE/ $(ROOT_DIR)bin/$(MODE)/CoreSys/init.re /EFI/UEFI/ $(ROOT_DIR)bin/Release/UEFI_Shell/SHELLX64.RE /EFI/FEUE/ $(ROOT_DIR)bin/$(MODE)/CoreSys/kernel.re /PU/ $(ROOT_DIR)bin/$(MODE)/CoreSys/c.re /EFI/GRUB/ $(ROOT_DIR).config/grub.cfg /EFI/LINUX/ $(ROOT_DIR)bin/Release/linux/bzImage /EFI/LINUX/ $(ROOT_DIR)build/linux/gz.gz /EFI/GRUB/ $(ROOT_DIR)bin/Release/GRUB/grubx64.re
endif

	@mv $(GPT_IMG) $(FINAL_IMG)
	@rm -f $(BUILD_EFI) $(GPT_FILE_TXT)
	@cp -f $(FINAL_IMG) $(DIST_LOCATION)
	@echo "CoreSys.img created at $(DIST_LOCATION)"

clean:
	@echo "Cleaning CoreSys build artifacts..."
	@$(MAKE) -f $(B_MAKE) clean
	@$(MAKE) -f $(K_MAKE) clean
	@$(MAKE) -f $(I_MAKE) clean
	@$(MAKE) -f $(C_MAKE) clean
	rm -f ./main.o
	@rm -f ./SEVI.TXT
	@rm -f ../../dist/.__smb008C
	@rm -f "$(FINAL_IMG)"
	