#!/bin/bash

# Check if argument is provided
if [ $# -lt 1 ]; then
  echo "Usage: $0 <MODE> (Debug/Release)"
  exit 1
fi

MODE="$1"

# Validate MODE
if [ "$MODE" != "Debug" ] && [ "$MODE" != "Release" ]; then
  echo "Error: MODE must be either 'Debug' or 'Release', not '$MODE'"
  exit 1
fi

IMG="../bin/$MODE/CoreSys/CoreSys.img"

# Check if image exists
if [ ! -f "$IMG" ]; then
  echo "Error: CoreSys image not found at $IMG"
  exit 1
fi

# Run QEMU
qemu-system-x86_64 \
-drive format=raw,file="$IMG" \
-bios ../firmware/bios64.bin \
-m 256M \
-vga std \
-name CoreSys \
-machine q35 \
-usb \
-device usb-mouse \
-rtc base=localtime \
-net none \
-serial stdio
