#!/bin/sh

qemu-system-x86_64 -bios ../../build/CIOS/cios.img -device isa-debugcon,chardev=cios -chardev stdio,id=cios
