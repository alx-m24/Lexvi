#!/usr/bin/env bash
set -e

usbipd.exe attach --wsl --busid 2-1

sleep 1.0
sudo dd if=build/lexvi.img of=/dev/sde bs=512 conv=fsync status=progress

sudo dd if=/dev/sde bs=1 skip=510 count=2 2>/dev/null | xxd
sleep 1.0
usbipd.exe detach --busid 2-1
