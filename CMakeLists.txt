cmake_minimum_required(VERSION 3.20)
project(LexviOS CXX ASM_NASM)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(GNUEFI_INC /usr/include/efi)
set(GNUEFI_LIB /usr/lib)

# Bootloader (UEFI app)
add_library(bootloader_obj OBJECT
    src/boot-uefi/main.cpp
    src/kernel/memory/pmm.cpp
    src/kernel/memory/vmm.cpp
    src/kernel/memory/memory-manager.cpp
)

target_compile_definitions(bootloader_obj PRIVATE BOOTLOADER)
target_compile_options(bootloader_obj PRIVATE
    -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fno-builtin
    -fno-exceptions -fno-rtti -fshort-wchar -mno-red-zone
    -maccumulate-outgoing-args
)
target_include_directories(bootloader_obj PRIVATE
    ${GNUEFI_INC}
    ${GNUEFI_INC}/x86_64
    include
)

set(BOOT_SO  ${CMAKE_BINARY_DIR}/BOOTX64.so)
set(BOOT_EFI ${CMAKE_BINARY_DIR}/BOOTX64.EFI)

add_custom_command(
    OUTPUT ${BOOT_SO}
    COMMAND ld -shared -Bsymbolic -L${GNUEFI_LIB}
        -T ${GNUEFI_LIB}/elf_x86_64_efi.lds
        ${GNUEFI_LIB}/crt0-efi-x86_64.o
        $<TARGET_OBJECTS:bootloader_obj>
        -o ${BOOT_SO}
        -lefi -lgnuefi
    DEPENDS bootloader_obj
    COMMENT "Linking bootloader (BOOTX64.so)"
    VERBATIM
    COMMAND_EXPAND_LISTS
)

add_custom_command(
    OUTPUT ${BOOT_EFI}
    COMMAND objcopy -j .text -j .sdata -j .data -j .dynamic
        -j .dynsym -j .rel -j .rela -j .reloc -j .rodata -j .bss
        --target=efi-app-x86_64
        ${BOOT_SO} ${BOOT_EFI}
    DEPENDS ${BOOT_SO}
    COMMENT "Converting bootloader to EFI PE image"
    VERBATIM
)

add_custom_target(bootx64 ALL DEPENDS ${BOOT_EFI})

# Kernel (flat binary, custom linker script)
add_executable(kernel
    src/kernel/memory/memory-manager.cpp
    src/kernel/console/tui/window.cpp
    src/kernel/keyboard/keyboard.cpp
    src/kernel/syscall/syscall.cpp
    src/kernel/console/console.cpp
    src/kernel/interrupt/idt.cpp
    src/kernel/interrupt/pic.cpp
    src/kernel/keyboard/ps2.cpp
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
    src/kernel/kernel.cpp

    src/kernel/interrupt/isr_stubs.nasm
    src/kernel/kernelMain-entry.nasm
)

set(CMAKE_CXX_LINK_EXECUTABLE
    "ld -m elf_x86_64 -T ${CMAKE_SOURCE_DIR}/link/kernel.ld <OBJECTS> -o <TARGET>")

target_compile_options(kernel PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-m64>
    $<$<COMPILE_LANGUAGE:CXX>:-ffreestanding>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-stack-protector>
    $<$<COMPILE_LANGUAGE:CXX>:-nostdlib>
    $<$<COMPILE_LANGUAGE:CXX>:-mno-red-zone>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    $<$<COMPILE_LANGUAGE:CXX>:-ffunction-sections>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-builtin>
)
target_include_directories(kernel PRIVATE
    include
    ${CMAKE_BINARY_DIR}/include
)
set_target_properties(kernel PROPERTIES
    OUTPUT_NAME "kernel"
    SUFFIX ".elf"
)

add_custom_command(TARGET kernel POST_BUILD
    COMMAND objcopy -O binary
        $<TARGET_FILE:kernel>
        $<TARGET_FILE_DIR:kernel>/kernel.bin
    COMMAND nm $<TARGET_FILE:kernel> | sort >
        $<TARGET_FILE_DIR:kernel>/kernel.map
    COMMENT "Converting kernel ELF to flat binary"
)

# ESP staging (what actually goes on the FAT image)
set(ESP_DIR ${CMAKE_BINARY_DIR}/esp)

add_custom_target(esp ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${ESP_DIR}/EFI/BOOT
    COMMAND ${CMAKE_COMMAND} -E copy ${BOOT_EFI} ${ESP_DIR}/EFI/BOOT/BOOTX64.EFI
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:kernel>/kernel.bin ${ESP_DIR}/kernel.bin
    DEPENDS bootx64 kernel
    COMMENT "Staging ESP (build/esp)"
)
