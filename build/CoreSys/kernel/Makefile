.PHONY: all clean FORCE gpt

# =====================
# Config file
# =====================

CONFIG_FILE ?= ../../.config/config.cfg

ifeq ("$(wildcard $(CONFIG_FILE))","")
$(error Missing config file: $(CONFIG_FILE))
endif

include $(CONFIG_FILE)

# =====================
# Ensure build dirs exist
# =====================

$(BUILD_DIR):
	mkdir -p $@

$(BIN_DIR):
	mkdir -p $@

# =====================
# CoreSys source files
# =====================

K_SRCS   := $(wildcard $(K_DIR)/*.c)
ASM_SRCS := $(wildcard $(K_DIR)/asm/*.s)

C_SRC := $(wildcard $(K_DIR)/*.c) \
         $(wildcard $(K_DIR)/library/ace/lib/source/*.c)

# =====================
# Objects
# =====================

C_OBJS := $(patsubst $(K_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SRC))

ASM_OBJS := $(patsubst $(K_DIR)/asm/%.s,$(BUILD_DIR)/asm/%.o,$(ASM_SRCS))

UTILS_SRC := $(wildcard $(K_DIR)/utils/*.s)
UTILS_OBJ := $(patsubst $(K_DIR)/utils/%.s,$(BUILD_DIR)/utils/%.o,$(UTILS_SRC))

ALL_OBJS := $(C_OBJS) $(ASM_OBJS)

# =====================
# Compile C
# =====================

$(BUILD_DIR)/%.o: $(K_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(K_CFLAGS) -c $< -o $@

# =====================
# Compile ASM
# =====================

$(BUILD_DIR)/asm/%.o: $(K_DIR)/asm/%.s
	@mkdir -p $(dir $@)
	nasm -f win64 $< -o $@

# =====================
# Compile Utils in ASM
# =====================

$(BUILD_DIR)/utils/%.o: $(K_DIR)/utils/%.s
	@mkdir -p $(dir $@)
	nasm -f bin $< -o $@

# =====================
# Link EFI
# =====================

$(BUILD_K_EFI): $(ALL_OBJS) | $(BIN_DIR)
	@echo "Linking CoreSys EFI..."
	$(CC) $(ALL_OBJS) $(K_LDFLAGS) -o $@
	@echo "Built EFI: $@"

$(BUILD_K_EFI_NE): $(BUILD_K_EFI) $(NE_TOOL)
	@echo "Applying NE header..."
	@$(NE_TOOL) -i $< -o $@

# =====================
# All
# =====================

all: $(BUILD_K_EFI) $(BUILD_K_EFI_NE)

# =====================
# Add raw ASM utilities to disk
# =====================

a: $(UTILS_OBJ)
	@if [ "$$($(ROOT_DIR)bin/Release/ata/ata.run $(ROOT_DIR)misc/disk/ata.img -v /bin)" = "1" ]; then \
		$(ROOT_DIR)bin/Release/ata/ata.run $(ROOT_DIR)misc/disk/ata.img -m bin -f; \
	fi
	$(ROOT_DIR)bin/Release/ata/ata.run $(ROOT_DIR)misc/disk/ata.img -k bin -f
	$(ROOT_DIR)bin/Release/ata/ata.run $(ROOT_DIR)misc/disk/ata.img -p $(ROOT_DIR)build/CoreSys/utils/test.o bin/it.bin

# =====================
# Clean
# =====================

clean:
	rm -fr "$(BUILD_DIR)/lib" "$(BUILD_DIR)/utils/" "$(BUILD_DIR)/asm/"
	rm -f $(ALL_OBJS) $(UTILS_OBJ) \
	      "$(BUILD_K_EFI)" \
	      "$(SRC_DIR/main.o)" \
	      "$(BUILD_DIR)/main.o" \
	      "$(SRC_DIR)/init/main.o" \
	      "$(BUILD_K_EFI_NE)"
	@echo "Cleaned build artifacts"
	