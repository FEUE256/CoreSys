#!/bin/sh

qemu-system-x86_64 -bios ./cios.img -device isa-debugcon,chardev=cios -chardev stdio,id=cios
