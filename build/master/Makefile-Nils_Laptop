.PHONY: all edk2 edk2_3 clean subdirs qemu

mc:
	python3 ../../scripts/maintenance/parser.py

# =====================
# Config files
# =====================
CONFIG_FILE := ../../.config/build.cfg
MODE_FILE   := ../../.config/config.cfg

all: mc clean subdirs

ifneq ("$(wildcard $(CONFIG_FILE))","")
include $(CONFIG_FILE)
else
$(error Missing config file: $(CONFIG_FILE))
endif

# -------- Subdirectories --------
subdirs:
	@for dir in $(SUBDIRS); do \
		if [ -f $$dir/Makefile ]; then \
			echo "[Make] Building in $$dir..."; \
			$(MAKE) -C $$dir all -B; \
		else \
			echo "[Make] No Makefile in $$dir, skipping"; \
		fi \
	done
	@if [ -f $(CORESYS_DIR)/Makefile ]; then \
		echo "[Make] Building CoreSys last..."; \
		$(MAKE) -C $(CORESYS_DIR) all -B; \
	else \
		echo "[Make] No Makefile in CoreSys, skipping"; \
	fi

# -------- Qemu ---------
qemu:
	@echo "Running QEMU..."
	@if [ ! -f ../../scripts/VM/qemu.sh ]; then \
		echo "Error: QEMU script not found at $(QEMU_SCRIPT)"; \
		exit 1; \
	fi
	@bash ../../scripts/VM/qemu.sh
	@echo "\n\nRan QEMU"

# -------- Clean --------
clean:
	@echo "Cleaning subdirectories..."
	@for dir in $(SUBDIRS); do \
		if [ -f "$$dir/Makefile" ]; then \
			echo "[Make] Cleaning in $$dir..."; \
			$(MAKE) -C "$$dir" clean -B; \
		fi; \
	done
	@rm -f ./CoreSys.img ./SEVI.TXT ../../bin/Debug/CoreSys/kernel.re
	