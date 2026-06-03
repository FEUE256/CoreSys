.PHONY: all edk2 edk2_3 clean subdirs qemu

mc:
	python3 ../../scripts/maintenance/parser.py

all: vm mc clean subdirs db

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

db:
	$(MAKE) -C ../db/ clean

vm:
	@mkdir -p $(HOME)/CoreSysVM

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

# -------- Qemu with GDB ---------
gdb:
	@echo "Running QEMU with GDB..."
	@if [ ! -f ../../scripts/VM/qemu_gdb.sh ]; then \
		echo "Error: QEMU GDB script not found at $(QEMU_GDB_SCRIPT)"; \
		exit 1; \
	fi
	@bash ../../scripts/VM/qemu_gdb.sh
	@echo "\n\nRan QEMU with GDB"

# -------- CIOS Qemu ---------
qc:
	@echo "Running CIOS QEMU..."
	@if [ ! -f ../../scripts/VM/qemu_cios.sh ]; then \
		echo "Error: QEMU script not found at ../../scripts/VM/qemu_cios.sh"; \
		exit 1; \
	fi
	@bash ../../scripts/VM/qemu_cios.sh
	@echo "\n\nRan CIOS QEMU"

cios:
	$(MAKE) -C ../CIOS/ all -B

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
	