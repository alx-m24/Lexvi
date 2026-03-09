#!/usr/bin/env bash
set -e

cd build
DISK_PATH=$(wslpath -w $(realpath lexvi-disk.vdi))
DISK_IMG=$(wslpath -w $(realpath lexvi.img))
FIXED_UUID="8e3db815-1520-4bcd-acc7-cba68a767269"

VBoxManage.exe closemedium disk "$FIXED_UUID" --delete 2>/dev/null || true
VBoxManage.exe convertfromraw "$DISK_IMG" "$DISK_PATH" --format VDI --uuid "$FIXED_UUID"
VBoxManage.exe storageattach "Lexvi" --storagectl "IDE" --port 0 --device 0 --type hdd --medium "$DISK_PATH"
