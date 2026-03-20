#!/usr/bin/env bash
set -e

# qemu-system-x86_64.exe -drive format=raw,file=$(wslpath -w build/lexvi.img)
# qemu-system-x86_64.exe \
#     -drive id=disk0,file=$(wslpath -w build/lexvi.img),format=raw,if=none \
#     -device ahci,id=ahci0 \
#     -device ide-hd,drive=disk0,bus=ahci0.0
qemu-system-x86_64.exe \
    -drive id=disk0,file=$(wslpath -w build/lexvi.img),format=raw,if=none \
    -device ahci,id=ahci0 \
    -device ide-hd,drive=disk0,bus=ahci0.0 \
    -m 256M \
    -no-reboot \
    -d int,cpu_reset 2>qemu.log
