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

all: edk2 $(if $(filter 1 true yes,$(DO_TEST)),edk2_3) subdirs

# -------- Main EDK2 tree --------
edk2:
ifeq ($(origin EDK2_MODE),environment)
	@echo "[EDK2] Skipping build because -e is used"
else
	@echo "[EDK2] Mode: $(EDK2_MODE)"
	$(MAKE) -C "$(EDK2_BUILD_DIR)"

	bash -c ' \
		set -e; \
		cd "$(EDK2_DIR)" && \
		source edksetup.sh --reconfig && \
		find . -name "*.dsc" | while read dsc; do \
			echo "[EDK2] Building $$dsc ($(EDK2_MODE))"; \
			build -t GCC5 -a X64 -b "$(EDK2_MODE)" -p "$$dsc" || true; \
		done \
	'
endif

# -------- Secondary / test tree --------
edk2_3:
ifeq ($(origin TEST_MODE),environment)
	@echo "[TEST] Skipping build because -e is used"
else
	@echo "[TEST] Mode: $(TEST_MODE)"
	$(MAKE) -C "$(BASETOOLS2_DIR)"

	bash -c ' \
		set -e; \
		cd "$(EDK23_DIR)" && \
		source edksetup.sh --reconfig && \
		find . -name "*.dsc" | while read dsc; do \
			echo "[TEST] Building $$dsc ($(TEST_MODE))"; \
			build -t GCC5 -a X64 -b "$(TEST_MODE)" -p "$$dsc" || true; \
		done \
	'
endif

# -------- Subdirectories --------
subdirs:
	# Bygg alla andra subdirs först
	@for dir in $(SUBDIRS); do \
		if [ -f $$dir/Makefile ]; then \
			echo "[Make] Building in $$dir..."; \
			$(MAKE) -C $$dir all -B; \
		else \
			echo "[Make] No Makefile in $$dir, skipping"; \
		fi \
	done
	# Bygg CoreSys sist
	@if [ -f $(CORESYS_DIR)/Makefile ]; then \
		echo "[Make] Building CoreSys last..."; \
		$(MAKE) -C $(CORESYS_DIR) all -B; \
	else \
		echo "[Make] No Makefile in CoreSys, skipping"; \
	fi

# -------- Qemu ---------
qemu:
	@echo "Running QEMU in $(QEMU_MODE) mode..."
	@if [ ! -f $(QEMU_SCRIPT) ]; then \
		echo "Error: QEMU script not found at $(QEMU_SCRIPT)"; \
		exit 1; \
	fi
	@(cd $(SCRIPTS_DIR) && bash qemu.sh $(QEMU_MODE))
	@echo "\n\nRan QEMU in $(QEMU_MODE) mode"

# -------- Clean --------
clean:
ifeq ($(EDK_NOT_CLEAN),1)
	@echo "Skipping EDK2 clean because EDK_NOT_CLEAN=1"
else
	@echo "Cleaning EDK2 and EDK23 builds..."
	rm -rf "$(EDK2_DIR)/Build" "$(EDK23_DIR)/Build"

	$(MAKE) -C "$(EDK2_BUILD_DIR)" clean
	$(MAKE) -C "$(BASETOOLS2_DIR)" clean

	cd "$(EDK2_DIR)" && rm -f ./*.efi
	cd "$(EDK23_DIR)" && rm -f ./*.efi
endif

	@echo "Cleaning subdirectories..."
	@for dir in $(SUBDIRS); do \
		if [ -f "$$dir/Makefile" ]; then \
			echo "[Make] Cleaning in $$dir..."; \
			$(MAKE) -C "$$dir" clean; \
		fi; \
	done
