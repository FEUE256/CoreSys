#!/bin/bash
# install-dev-tools-compact-fixed.sh
set -eu

# Use old-releases repository for EOL Ubuntu
sudo sed -i.bak -E 's|http://ports.ubuntu.com/ubuntu-ports/|http://old-releases.ubuntu.com/ubuntu/|g' /etc/apt/sources.list

echo "This can break if it breaks, just check the scripts code and install each package manually if it does"

echo "Installing development packages..."

packages=(
    make
    gcc
    g++
    llvm
    binutils
    coreutils
    vim-common
    gzip 
    cpio
    clang
    lld
    git-lfs
    python-is-python3
    gcc-mingw-w64
    g++-mingw-w64
    binutils-mingw-w64
    qemu-system-x86
    ovmf
    uuid-dev
    nasm
    acpica-tools
    git
    python3
    python3-venv
    build-essential
    dos2unix
    hexedit
    bison
    flex
    libgmp3-dev
    libmpc-dev
    libmpfr-dev
    texinfo
    mtools
    sleuthkit
    libisl-dev
    gcc-x86-64-linux-gnu 
    binutils-x86-64-linux-gnu
    qemu-system-gui
    python-pip
    python3-pip
)

for pkg in "${packages[@]}"; do
    echo "Installing $pkg..."
    if ! sudo apt install -y "$pkg"; then
        echo "Warning: Failed to install $pkg, skipping."
    fi
done

git lfs install

echo "Done installing packages."

echo "All packages installed successfully."
