#!/bin/bash
# install-dev-tools-fixed2.sh
# Säker version utan tomma paket

set -e
set -u

echo "Updating package lists..."
sudo apt update

echo "Installing development packages..."
sudo apt install -y \
    make \
    gcc \
    g++ \
    llvm \
    clang \
    lld \
    python-is-python3 \
    gcc-mingw-w64 \
    g++-mingw-w64 \
    binutils-mingw-w64 \
    qemu-system-x86 \
    ovmf \
    uuid-dev \
    nasm \
    acpica-tools \
    git \
    python3 \
    python3-venv \
    build-essential \
    dos2unix \
    hexedit \
    bison \
    flex \
    libgmp3-dev \
    libmpc-dev \
    libmpfr-dev \
    texinfo \
    mtools \
    sleuthkit \
    libisl-dev

echo "All packages installed successfully."
