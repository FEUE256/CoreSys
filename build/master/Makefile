.PHONY: all edk2 edk2_3 clean subdirs qemu

mc:
	python3 "../../scripts/maintenance/parser.py"

all: vm mc clean subdirs home

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

ip:
	@if ip link show tap0 > /dev/null 2>&1; then \
		sudo ip link delete tap0; \
	fi; \
	sudo ip tuntap add dev tap0 mode tap; \
	sudo ip link set tap0 up

vm:
	@mkdir -p $(HOME)/CoreSysVM

home:
	@cp ../../dist/CoreSys.img $(HOME)/CoreSysVM/CoreSys.img

# -------- Subdirectories --------
subdirs:
	@for dir in $(SUBDIRS); do \
		if [ -f $$dir/Makefile ]; then \
			echo "[Make] Building in $$dir..."; \
			$(MAKE) -C $$dir all NO_NPM=$(NO_NPM) -B; \
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

# -------- A Qemu ---------
a:
	@echo "Running A QEMU..."
	@if [ ! -f ../../scripts/VM/a.sh ]; then \
		echo "Error: QEMU script not found at ../../scripts/VM/a.sh"; \
		exit 1; \
	fi
	@sudo bash ../../scripts/VM/a.sh
	@echo "\n\nRan CIOS QEMU"

cios:
	$(MAKE) -C ../CIOS/ all 

# -------- Clean --------
clean:
	@echo "Cleaning subdirectories..."
	@for dir in $(SUBDIRS); do \
		if [ -f "$$dir/Makefile" ]; then \
			echo "[Make] Cleaning in $$dir..."; \
			$(MAKE) -C "$$dir" clean ; \
		fi; \
	done
	@rm -f ./CoreSys.img ./SEVI.TXT ../../bin/Debug/CoreSys/kernel.re
	