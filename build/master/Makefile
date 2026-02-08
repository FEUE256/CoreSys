.PHONY: all clean

# =====================
# Config files
# =====================
CONFIG_FILE := ../../.config/build.cfg

ifneq ("$(wildcard $(CONFIG_FILE))","")
include $(CONFIG_FILE)
else
$(error Missing config file: $(CONFIG_FILE))
endif

all:
	@for dir in $(SUBDIRS); do \
		if [ -f $$dir/Makefile ]; then \
			echo "[Make] Building in $$dir..."; \
			$(MAKE) all -B -C $$dir; \
		else \
			echo "[Make] No Makefile in $$dir, skipping"; \
		fi \
	done

	$(MAKE) all -B -C 

clean:
	@for dir in $(SUBDIRS); do \
		if [ -f $$dir/Makefile ]; then \
			echo "[Make] Cleaning in $$dir..."; \
			$(MAKE) -C $$dir clean; \
		fi \
	done
