#!/bin/bash

# Corrected path to CoreSys image
IMG="../../dist/CoreSys.img"

if [ ! -f "$IMG" ]; then
  echo "Error: CoreSys image not found at $IMG"
  exit 1
fi

qemu-system-x86_64 \
-drive format=raw,file="$IMG" \
-bios ../../firmware/bios/bios64.bin \
-m 256M \
-vga std \
-name CoreSys \
-machine q35 \
-usb \
-device usb-mouse \
-rtc base=localtime \
-net none \
-serial stdio
