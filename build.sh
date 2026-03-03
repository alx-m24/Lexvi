#!/usr/bin/env bash

# clearing output path
rm -rf build
mkdir -p build

# building bootloader
nasm -f bin src/boot-stage1.nasm -o build/boot-stage1.bin
nasm -f bin src/boot-stage2.nasm -o build/boot-stage2.bin

cd build

# 1. Create a blank 1.44MB floppy image
dd if=/dev/zero of=disk.img bs=512 count=2880

# 2. Write bootloader to the first sector
dd if=boot-stage1.bin of=disk.img bs=512 count=1 conv=notrunc

#3. Write boot-stage2 to second sector
dd if=boot-stage2.bin of=disk.img bs=512 seek=1 conv=notrunc
