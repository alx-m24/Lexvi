#!/usr/bin/env bash
set -e

source scripts/color.sh
mkdir -p build/

print_color white "Starting build..."
print_color yellow "Building bootloader..."

GNUEFI_INC=/usr/include/efi
GNUEFI_LIB=/usr/lib

gcc -I$GNUEFI_INC -I$GNUEFI_INC/x86_64 \
    -fpic -ffreestanding -fno-stack-protector -fno-stack-check \
    -fshort-wchar -mno-red-zone -maccumulate-outgoing-args \
    -c src/boot-uefi/main.cpp -o build/uefi-main.o \
    -I include \
    -std=c++23

print_color green "Successfully built bootloader..."
print_color yellow "Linking bootloader..."

ld -shared -Bsymbolic -L$GNUEFI_LIB \
    -T $GNUEFI_LIB/elf_x86_64_efi.lds \
    $GNUEFI_LIB/crt0-efi-x86_64.o \
    build/uefi-main.o \
    -o build/BOOTX64.so \
    -lefi -lgnuefi

print_color green "Successfully linked bootloader..."

objcopy -j .text -j .sdata -j .data -j .dynamic \
    -j .dynsym -j .rel -j .rela -j .reloc -j .rodata -j .bss \
    --target=efi-app-x86_64 \
    build/BOOTX64.so build/BOOTX64.EFI

print_color cyan "Copying bootloader"

mkdir -p build/esp/EFI/BOOT/
cp build/BOOTX64.EFI build/esp/EFI/BOOT/

print_color yellow "Building kernel..."

cmake -S . -B build/cmake -G "Unix Makefiles" 2>/dev/null
cmake --build build/cmake --target kernel

rm -rf build/compile_commands.json
ln -s cmake/compile_commands.json build/compile_commands.json

print_color green "Successfully built kernel..."

print_color yellow "Copying kernel for FAT..."

cp build/cmake/kernel.bin build/esp/

print_color yellow "Successfully copied kernel for FAT..."

print_color green "Successfully built Lexvi OS"
