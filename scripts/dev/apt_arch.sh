#!/bin/bash
set -euo pipefail

echo "Updating system..."
sudo pacman -Syu --noconfirm

echo "Installing development packages..."

packages=(
    base-devel
    gcc
    clang
    llvm
    lld
    binutils
    gzip
    cpio
    make

    python
    python-pip

    mingw-w64-gcc
    mingw-w64-binutils

    qemu-full
    edk2-ovmf

    util-linux
    nasm
    acpica

    git

    dos2unix
    hexedit

    bison
    flex
    texinfo

    gmp
    mpc
    mpfr
    isl

    mtools
    sleuthkit

    binutils
)

for pkg in "${packages[@]}"; do
    echo "Installing $pkg..."
    if ! sudo pacman -S --needed --noconfirm "$pkg"; then
        echo "Warning: Failed to install $pkg, skipping."
    fi
done

git lfs install

echo "Done installing packages."
