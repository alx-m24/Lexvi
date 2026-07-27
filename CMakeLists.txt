cmake_minimum_required(VERSION 3.20)
project(kernel CXX ASM_NASM)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_LINK_EXECUTABLE
    "ld -m elf_x86_64 -T ${CMAKE_SOURCE_DIR}/link/kernel.ld <OBJECTS> -o <TARGET>")

add_executable(kernel
    src/kernel/memory/memory-manager.cpp
    src/kernel/console/tui/window.cpp
    src/kernel/keyboard/keyboard.cpp
    src/kernel/syscall/syscall.cpp
    src/kernel/console/console.cpp
    src/kernel/interrupt/idt.cpp
    src/kernel/utils/memory.cpp
    src/kernel/kernel-main.cpp
    src/kernel/error/error.cpp
    src/kernel/memory/pmm.cpp
    src/kernel/memory/vmm.cpp
    src/kernel/time/time.cpp
    src/kernel/acpi/rsdp.cpp
    src/kernel/acpi/hpet.cpp
    src/kernel/acpi/sdt.cpp
    src/kernel/gdt/gdt.cpp
    src/kernel/pic/pic.cpp
    src/kernel/kernel.cpp

    src/kernel/interrupt/isr_stubs.nasm
    src/kernel/kernelMain-entry.nasm
)

target_compile_options(kernel PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-m64>
    $<$<COMPILE_LANGUAGE:CXX>:-ffreestanding>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-stack-protector>
    $<$<COMPILE_LANGUAGE:CXX>:-nostdlib>
    $<$<COMPILE_LANGUAGE:CXX>:-mno-red-zone>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    $<$<COMPILE_LANGUAGE:CXX>:-ffunction-sections>
)

target_include_directories(kernel PRIVATE
    include
    build/include
)

set_target_properties(kernel PROPERTIES
    OUTPUT_NAME "kernel"
    SUFFIX ".elf"
)

# Post-build: convert ELF to flat binary
add_custom_command(TARGET kernel POST_BUILD
    COMMAND objcopy -O binary
        $<TARGET_FILE:kernel>
        $<TARGET_FILE_DIR:kernel>/kernel.bin
    COMMAND nm $<TARGET_FILE:kernel> | sort >
        $<TARGET_FILE_DIR:kernel>/kernel.map
    COMMENT "Converting ELF to flat binary"
)
