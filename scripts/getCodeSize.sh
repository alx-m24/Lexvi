#!/usr/bin/env bash

if [ -z "$1" ]; then
    echo "Usage: getCodeSize <filePath>"
    exit 1
fi

filePath=$(realpath "$1")
filename=$(basename "$filePath")
extension="${filePath##*.}"

if [[ "$extension" != "nasm" && "$extension" != "asm" ]]; then
    echo "$filePath is not an assembly file"
    exit 1
fi

mkdir -p temp && cd temp

nasm -f bin "${filePath}" -o "${filename}.bin"

FILE_SIZE=$(stat -c %s "${filename}.bin")
echo "File size in bytes: $FILE_SIZE"

cd ..
rm -rf temp
