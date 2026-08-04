#!/usr/bin/env bash
set -e

OVMF_CODE=$(wslpath -w /usr/share/OVMF/OVMF_CODE_4M.fd)

if [ ! -f build/OVMF_VARS.fd ]; then
    cp /usr/share/OVMF/OVMF_VARS_4M.fd build/OVMF_VARS.fd
fi
OVMF_VARS=$(wslpath -w build/OVMF_VARS.fd)

qemu-system-x86_64.exe \
    -drive if=pflash,format=raw,readonly=on,file="$OVMF_CODE" \
    -drive if=pflash,format=raw,file="$OVMF_VARS" \
    -drive file=fat:rw:$(wslpath -w build/esp),format=raw \
    -m 8G \
    -no-reboot \
    -d int,cpu_reset \
    -chardev vc,id=char0,logfile=log/serial.log \
    -serial chardev:char0 \
    2>log/qemu.log
