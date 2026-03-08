#!/usr/bin/env bash
set -e

# clearing output path
rm -rf build
mkdir -p build

# ---- Pass 1 ----

# building bootloader
nasm -f bin src/boot/boot-stage1.nasm -o build/boot-stage1.bin -I include/
nasm -f elf64 src/boot/boot-stage2.nasm -o build/boot-stage2.o

# building kernel files
g++ -m64 -ffreestanding -fno-stack-protector -nostdlib -mno-red-zone -fno-exceptions -fno-rtti -c src/kernel/kernel-entry.cpp -o build/kernel-entry.o -I include

# building kernel
g++ -m64 -ffreestanding -fno-stack-protector -nostdlib -mno-red-zone \
    -fno-exceptions -fno-rtti \
    -ffunction-sections \
    -c src/kernel/kernel-main.cpp -o build/kernel-main.o

# linking stage 2 and kernel
ld -m elf_x86_64 -T link/kernelEntry.ld build/boot-stage2.o build/kernel-entry.o -o build/boot-stage2.bin --oformat binary
ld -m elf_x86_64 -T link/kernelMain.ld build/kernel-main.o -o build/kernel-main.bin --oformat binary

# --- Getting sizes ---
STAGE2_SIZE=$(stat -c%s "build/boot-stage2.bin")
KERNEL_MAIN_SIZE=$(stat -c%s "build/kernel-main.bin")

STAGE2_SECTORS=$((("$STAGE2_SIZE" +511) / 512))
KERNEL_MAIN_SECTORS=$((("$KERNEL_MAIN_SIZE" +511) / 512))

KERNEL_MAIN_LBA=$((STAGE2_SECTORS + 1))

# --- Pass 2 / Final Pass ---
rm -rf build
mkdir -p build

cat > include/asm/boot-config.nasm << EOF 
STAGE2_SECTORS equ ${STAGE2_SECTORS} 
EOF

cat > include/kernel/kernel-config.hpp << EOF
#pragma once

constexpr unsigned int KERNEL_MAIN_LBA = ${KERNEL_MAIN_LBA};
constexpr unsigned int KERNEL_MAIN_SECTORS = ${KERNEL_MAIN_SECTORS};
constexpr unsigned long long KERNEL_MAIN_LOAD_ADDR = 0x100000;
EOF

# building bootloader
nasm -f bin src/boot/boot-stage1.nasm -o build/boot-stage1.bin -I include/
nasm -f elf64 src/boot/boot-stage2.nasm -o build/boot-stage2.o

# building kernel files
g++ -m64 -ffreestanding -fno-stack-protector -nostdlib -mno-red-zone -fno-exceptions -fno-rtti -c src/kernel/kernel-entry.cpp -o build/kernel-entry.o -I include

# building kernel
g++ -m64 -ffreestanding -fno-stack-protector -nostdlib -mno-red-zone \
    -fno-exceptions -fno-rtti \
    -ffunction-sections \
    -c src/kernel/kernel-main.cpp -o build/kernel-main.o

# linking stage 2 and kernel
ld -m elf_x86_64 -T link/kernelEntry.ld build/boot-stage2.o build/kernel-entry.o -o build/boot-stage2.bin --oformat binary
ld -m elf_x86_64 -T link/kernelMain.ld build/kernel-main.o -o build/kernel-main.bin --oformat binary

# Writing to disk

cd build

# 1. Create a blank 1.44MB floppy image
dd if=/dev/zero of=disk.img bs=512 count=20480

# 2. Write bootloader to the first sector
dd if=boot-stage1.bin of=disk.img bs=512 count=1 conv=notrunc

#3. Write boot-stage2 to second sector
dd if=boot-stage2.bin of=disk.img bs=512 seek=1 conv=notrunc

#4. Write kernel starting from sector 8
dd if=kernel-main.bin of=disk.img bs=512 seek=${KERNEL_MAIN_LBA} conv=notrunc
