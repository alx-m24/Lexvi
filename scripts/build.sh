#!/usr/bin/env bash
set -e

source scripts/color.sh

# cmake -DCMAKE_BUILD_TYPE=Release -S . -B build -G "Unix Makefiles"
cmake -S . -B build -G "Unix Makefiles"
cmake --build build

print_color green "Successfully built Lexvi OS"
