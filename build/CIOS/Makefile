# =====================
# Config files
# =====================
CONFIG_FILE := ../../.config/cios.cfg

ifneq ("$(wildcard $(CONFIG_FILE))","")
include $(CONFIG_FILE)
else
$(error Missing config file: $(CONFIG_FILE))
endif

code:
	nasm -f bin $(SRC)/cios.s -o ./cios.bin

img:
	truncate -s 4M ./cios.img

	xxd -p -c 1 cios.bin | tr -d '\n' | tr 'a-f' 'A-F' | xxd -r -p > cios.tmp
	dd if=cios.tmp of=cios.img bs=1 seek=$$((4*1024*1024-16)) conv=notrunc

	cp ./cios.img $(DIST)/cios.img

all: clean code img

run:	
	bash $(VM)/qemu.sh

clean: 
	rm -rf ./cios.img ./cios.bin ./init.bin ./cios.tmp ./init.tmp
