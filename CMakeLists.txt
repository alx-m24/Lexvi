cmake_minimum_required(VERSION 3.20)
project(KernelMain CXX ASM_NASM)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# NASM output format
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> -f elf64 -o <OBJECT> <SOURCE>")

set(CMAKE_CXX_LINK_EXECUTABLE
    "ld -m elf_x86_64 -T ${CMAKE_SOURCE_DIR}/link/kernelMain.ld <OBJECTS> -o <TARGET>")

add_executable(kernel-main
    src/kernel/console/tui/window.cpp
    src/kernel/keyboard/keyboard.cpp
    src/kernel/syscall/syscall.cpp
    src/kernel/console/console.cpp
    src/kernel/interrupt/idt.cpp
    src/kernel/kernel-main.cpp
    src/kernel/error/error.cpp
    src/kernel/memory/pmm.cpp
    src/kernel/time/time.cpp
    src/kernel/acpi/rsdp.cpp
    src/kernel/acpi/sdt.cpp
    src/kernel/gdt/gdt.cpp
    src/kernel/pic/pic.cpp
    src/kernel/kernel.cpp

    src/kernel/interrupt/isr_stubs.nasm
    src/kernel/kernelMain-entry.nasm
)

target_compile_options(kernel-main PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-m64>
    $<$<COMPILE_LANGUAGE:CXX>:-ffreestanding>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-stack-protector>
    $<$<COMPILE_LANGUAGE:CXX>:-nostdlib>
    $<$<COMPILE_LANGUAGE:CXX>:-mno-red-zone>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    $<$<COMPILE_LANGUAGE:CXX>:-ffunction-sections>
)

target_include_directories(kernel-main PRIVATE
    include
    build/include
)

set_target_properties(kernel-main PROPERTIES
    OUTPUT_NAME "kernel-main"
    SUFFIX ".elf"
)

# Post-build: convert ELF to flat binary
add_custom_command(TARGET kernel-main POST_BUILD
    COMMAND objcopy -O binary
        $<TARGET_FILE:kernel-main>
        $<TARGET_FILE_DIR:kernel-main>/kernel-main.bin
    COMMAND nm $<TARGET_FILE:kernel-main> | sort >
        $<TARGET_FILE_DIR:kernel-main>/kernel-main.map
    COMMENT "Converting ELF to flat binary"
)
