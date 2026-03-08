cmake_minimum_required(VERSION 3.20)
project(KernelMain CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tell CMake to use ld directly, not the compiler for linking
set(CMAKE_CXX_LINK_EXECUTABLE
    "ld -m elf_x86_64 -T ${CMAKE_SOURCE_DIR}/link/kernelMain.ld --oformat binary <OBJECTS> -o <TARGET>")

add_executable(kernel-main
    src/kernel/kernel-main.cpp
    src/kernel/io/console.cpp
)

target_compile_options(kernel-main PRIVATE
    -m64
    -ffreestanding
    -fno-stack-protector
    -nostdlib
    -mno-red-zone
    -fno-exceptions
    -fno-rtti
    -ffunction-sections
)

target_include_directories(kernel-main PRIVATE
    include
    build/include
)

set_target_properties(kernel-main PROPERTIES
    OUTPUT_NAME "kernel-main"
    SUFFIX ".bin"
)
