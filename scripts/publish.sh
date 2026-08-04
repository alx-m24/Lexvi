#!/usr/bin/env bash
set -e

source scripts/color.sh

rm -rf build/esp.img
rm -rf build/lexvi.img

print_color "blue" "--> Creating blank ESP partition image (64MB)..."
dd if=/dev/zero of=build/esp.img bs=1M count=64 status=none

print_color "blue" "--> Formatting ESP image as FAT32..."
mkfs.vfat -F 32 build/esp.img

print_color "blue" "--> Populating ESP directory structure and files..."
mmd -i build/esp.img ::/EFI
mmd -i build/esp.img ::/EFI/BOOT
mcopy -i build/esp.img build/esp/EFI/BOOT/BOOTX64.EFI ::/EFI/BOOT/BOOTX64.EFI
mcopy -i build/esp.img build/esp/kernel.bin ::/kernel.bin

print_color "green" "--> ESP populated successfully. Contents:"
mdir -i build/esp.img ::/EFI/BOOT

print_color "blue" "--> Creating target disk image (128MB)..."
dd if=/dev/zero of=build/lexvi.img bs=1M count=128 status=none

print_color "blue" "--> Writing GPT header & setting up EFI partition..."
sgdisk -o build/lexvi.img > /dev/null
sgdisk -n 1:0:0 -t 1:ef00 -c 1:"EFI System" build/lexvi.img > /dev/null

print_color "blue" "--> Extracting first sector of partition 1..."
# Fix: Parse the specific "First sector:" field into a clean integer
firstSector=$(sgdisk -i 1 build/lexvi.img | awk '/First sector:/ {print $3}')

print_color "green" "    First sector located at: ${firstSector}"

print_color "blue" "--> Embedding ESP into disk image..."
# Fix: Removed literal angle brackets < > around firstSector
dd if=build/esp.img of=build/lexvi.img bs=512 seek=${firstSector} conv=notrunc status=none

print_color "green" "--> Disk image successfully built at build/lexvi.img!"
