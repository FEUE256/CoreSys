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
  -drive format=raw,file="../../dist/CoreSys.img" \
  -drive file=$HOME/CoreSysVM/disk.img,format=raw,if=ide \
  -bios "$HOME/CoreSysVM/bios64.bin" \
  -m 256M \
  -machine q35 \
  -name CoreSys \
  -usb \
  -device qemu-xhci \
  -device usb-tablet \
  -device usb-kbd \
  -rtc base=localtime \
  -boot order=c \
  -netdev tap,id=net0,ifname=tap0,script=no,downscript=no \
  -device e1000,netdev=net0 \
  -serial mon:stdio \
  -audiodev wav,id=audio0,path="$HOME/CoreSysVM/sound.wav" # \
  # -nographic 
