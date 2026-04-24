# FIXED QEMU LAUNCH SCRIPT (robust version)

set -e

IMG="../../dist/CoreSys.img"

# --- validate image ---
if [ ! -f "$IMG" ]; then
  echo "Error: CoreSys image not found at $IMG"
  exit 1
fi

BIOS_PATH="../../firmware/bios/bios64.bin"

mkdir -p $HOME/CoreSysVM

cp ../../dist/CoreSys.img $HOME/CoreSysVM/CoreSys.img
cp $BIOS_PATH $HOME/CoreSysVM/bios64.bin

# --- run QEMU ---
qemu-system-x86_64 \
  -drive format=raw,file="$HOME/CoreSysVM/CoreSys.img" \
  -bios "$HOME/CoreSysVM/bios64.bin" \
  -m 256M \
  -machine q35 \
  -name CoreSys \
  -usb \
  -device usb-mouse \
  -rtc base=localtime \
  -netdev user,id=n0 \
  -device rtl8139,netdev=n0 \
  -serial stdio \
  -display none \
  -audiodev wav,id=audio0,path="$HOME/CoreSysVM/sound.wav"
