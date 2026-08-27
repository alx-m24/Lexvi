#!/usr/bin/env bash
set -e

source scripts/color.sh

buildType="Debug"
if [ -n "$1" ]; then
    case "$1" in
        "debug") 
            buildType="Debug"
            ;;
        "release")
            buildType="Release"
            ;;
        *)
             echo "Build type can only be 'debug' or 'release'"
             exit 1
            ;;
    esac
fi

cmake -DCMAKE_BUILD_TYPE="${buildType}" -S . -B build -G "Unix Makefiles"
cmake --build build

print_color green "Successfully built Lexvi OS"
