#!/usr/bin/env bash
set -e

source scripts/color.sh

buildDir="build/"
ESP_DIR="${buildDir}/esp"
OVMF_VARS="${buildDir}/OVMF_VARS.fd"

OVMF_CODE=$(wslpath -w /usr/share/OVMF/OVMF_CODE_4M.fd)

if [ ! -f "$OVMF_VARS" ]; then
    cp /usr/share/OVMF/OVMF_VARS_4M.fd "$OVMF_VARS"
fi

OVMF_VARS=$(wslpath -w "$OVMF_VARS")
ESP_DIR=$(wslpath -w "$ESP_DIR")

mkdir -p log

qemu-system-x86_64.exe \
    -machine q35 \
    -drive if=pflash,format=raw,readonly=on,file="$OVMF_CODE" \
    -drive if=pflash,format=raw,file="$OVMF_VARS" \
    -drive file=fat:rw:"$ESP_DIR",format=raw \
    -m 8G \
    -no-reboot \
    -d int,cpu_reset \
    -chardev vc,id=char0,logfile="log/serial.log" \
    -serial chardev:char0 \
    2>log/qemu.log
