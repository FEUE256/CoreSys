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
nice -21 qemu-system-x86_64 \
  -drive format=raw,file="../../misc/disk/ata.img",if=ide,index=1 \
  -drive format=raw,file="../../dist/CoreSys.img",if=ide,index=0 \
  -drive file="../../misc/disk/nvme.img",if=none,id=nvmedisk,format=raw \
  -device nvme,drive=nvmedisk,serial=DISK001,physical_block_size=512,logical_block_size=512 \
  -bios "$HOME/CoreSysVM/bios64.bin" \
  -m 256 \
  -machine pc \
  -name CoreSys \
  -usb \
  -device qemu-xhci \
  -device usb-tablet \
  -device usb-kbd \
  -rtc base=localtime \
  -boot order=c \
  -netdev tap,id=net0,ifname=tap0,script=no,downscript=no \
  -device e1000,netdev=net1 \
  -netdev user,id=net1 \
  -device e1000,netdev=net0 \
  -device pcie-root-port,id=rp1 \
  -device virtio-balloon-pci,bus=rp1 \
  -serial mon:stdio \
  -serial /dev/ttyS3 \
  -vga std \
  -cpu max,+avx,enforce \
  -audiodev wav,id=audio0,path="$HOME/CoreSysVM/sound.wav" # \
  # -nographic 
