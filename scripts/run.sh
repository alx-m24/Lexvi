#!/usr/bin/env bash
set -e

qemu-system-x86_64.exe -drive format=raw,file=$(wslpath -w build/lexvi.img)
