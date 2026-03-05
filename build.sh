#!/usr/bin/env bash

# clearing output path
rm -rf build
mkdir -p build

# building bootloader
nasm -f bin src/boot/boot-stage1.nasm -o build/boot-stage1.bin
nasm -f elf64 src/boot/boot-stage2.nasm -o build/boot-stage2.o

# building kernel files
# gpp -m64 -ffreestanding -fno-stack-protector -nostdlib -mno-red-zone -c src/kernel/kernel-entry.c -o build/kernel-entry.o
g++ -m64 -ffreestanding -fno-stack-protector -nostdlib -mno-red-zone -fno-exceptions -fno-rtti -c src/kernel/kernel-entry.cpp -o build/kernel-entry.o

# linking stage 2 and kernel
ld -m elf_x86_64 -T link.ld build/boot-stage2.o build/kernel-entry.o -o build/boot-stage2.bin --oformat binary

cd build

# 1. Create a blank 1.44MB floppy image
dd if=/dev/zero of=disk.img bs=512 count=2880

# 2. Write bootloader to the first sector
dd if=boot-stage1.bin of=disk.img bs=512 count=1 conv=notrunc

#3. Write boot-stage2 to second sector
dd if=boot-stage2.bin of=disk.img bs=512 seek=1 conv=notrunc
