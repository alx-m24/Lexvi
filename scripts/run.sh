#!/usr/bin/env bash
set -e

# qemu-system-x86_64.exe -display sdl,grab-on-hover=on -drive format=raw,file=$(wslpath -w build/lexvi.img)
qemu-system-x86_64.exe -display sdl,grab-on-hover=on,show-cursor=off \
  -drive format=raw,file=$(wslpath -w build/lexvi.img) \
  -k en-us
