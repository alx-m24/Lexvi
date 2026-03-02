#!/usr/bin/env bash

# clearing output path
rm -rf build
mkdir -p build

# building bootloader
nasm -f bin src/boot.nasm -o build/boot.bin

cd build

# 1. Create a blank 1.44MB floppy image
dd if=/dev/zero of=disk.img bs=512 count=2880

# 2. Write your bootloader to the first sector
dd if=boot.bin of=disk.img bs=512 count=1 conv=notrunc
