CC = gcc
CFLAGS = -static -O2 -Wno-unused-result
ROOT = ../../
AS = as
SRC = $(ROOT)src/linux/
NE_MODE = Release
GRUB_MODE = Release

all: clean init cpio

sys.o: $(SRC)sys.s
	$(AS) $(SRC)sys.s -o sys.o

init.o: $(SRC)init.c
	$(CC) $(CFLAGS) -c $(SRC)init.c -o init.o

init: sys.o init.o
	$(CC) -static init.o sys.o -o init

cpio:
	bash -x cpio.sh // BASH ONLY 
	echo $?

clean:
	rm -rf init init.o sys.o files init.cpio gz.gz test/ root/ lua utils_bin
	