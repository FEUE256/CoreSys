.PHONY: all edk2 edk2_3 clean subdirs qemu

# =====================
# Config files
# =====================
CONFIG_FILE := ../../.config/build.cfg
MODE_FILE   := ../../.config/config.cfg

ifneq ("$(wildcard $(CONFIG_FILE))","")
include $(CONFIG_FILE)
else
$(error Missing config file: $(CONFIG_FILE))
endif

ifneq ("$(wildcard $(MODE_FILE))","")
include $(MODE_FILE)
else
$(error Missing mode config file: $(MODE_FILE))
endif

ifndef EDK2_DIR
$(error EDK2_DIR is not set)
endif

ifndef EDK2_BUILD_DIR
$(error EDK2_BUILD_DIR is not set)
endif

ifndef EDK23_DIR
$(error EDK23_DIR is not set)
endif

ifndef BASETOOLS2_DIR
$(error BASETOOLS2_DIR is not set)
endif

all: subdirs

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
	@if [ ! -f $(QEMU_SCRIPT) ]; then \
		echo "Error: QEMU script not found at $(QEMU_SCRIPT)"; \
		exit 1; \
	fi
	@(cd $(SCRIPTS_DIR) && ./qemu.sh)
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
