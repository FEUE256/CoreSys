#!/bin/sh
# Launch CoreSys in QEMU
# Usage: ./qemu.sh [debug|release]

MODE="${1:-release}"

case "$MODE" in
    debug)
        NAME="Debug"
        ;;
    release)
        NAME="Release"
        ;;
    *)
        echo "Invalid mode: $MODE"
        echo "Valid modes: debug, release"
        exit 1
        ;;
esac

BOOT_IMG="../bin/$NAME/CoreSys/boot.img"
FIRMWARE="../firmware/OVMF_CODE.fd"

# Check if boot image exists
if [ ! -f "$BOOT_IMG" ]; then
    echo "Error: Boot image not found at $BOOT_IMG"
    echo "Please build the project first."
    exit 1
fi

# Check if firmware exists (optional)
BIOS_ARGS=""
if [ -f "$FIRMWARE" ]; then
    BIOS_ARGS="-bios $FIRMWARE"
fi

qemu-system-x86_64 \
    -m 256M \
    -machine q35 \
    -drive format=raw,file="$BOOT_IMG" \
    $BIOS_ARGS \
    -name "CoreSys ($MODE)" \
    -netdev user,id=net0 \
    -device e1000,netdev=net0 \
    -serial stdio \
    -monitor vc