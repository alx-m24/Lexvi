cmake_minimum_required(VERSION 3.25)
project(LexviOS CXX ASM_NASM)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(GNUEFI_INC /usr/include/efi)
set(GNUEFI_LIB /usr/lib)

# Platform-specific compilation
set(LEXVI_PLATFORM "GENERIC" CACHE STRING "Target platform")

if(LEXVI_PLATFORM STREQUAL "INTEL")
    add_definitions(-DLEXVI_INTEL)

elseif(LEXVI_PLATFORM STREQUAL "AMD")
    add_definitions(-DLEXVI_AMD)

elseif(LEXVI_PLATFORM STREQUAL "AUTO")
    add_definitions(-DLEXVI_INTEL)
    add_definitions(-DLEXVI_AMD)

endif()

# common files
file(GLOB_RECURSE COMMON_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/common/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/*.asm"
    "${CMAKE_SOURCE_DIR}/src/common/*.nasm"
)

# bootloader files
file(GLOB_RECURSE BOOTLOADER_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/boot/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/boot/*.asm"
    "${CMAKE_SOURCE_DIR}/src/boot/*.nasm"
)

# UEFI App (bootloader)
add_library(bootloader_obj OBJECT
    ${BOOTLOADER_SOURCES}
    ${COMMON_SOURCES}
)

target_compile_definitions(bootloader_obj PRIVATE BOOTLOADER)
target_compile_options(bootloader_obj PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-m64>
    $<$<COMPILE_LANGUAGE:CXX>:-ffreestanding>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-stack-protector>
    $<$<COMPILE_LANGUAGE:CXX>:-nostdlib>
    $<$<COMPILE_LANGUAGE:CXX>:-mno-red-zone>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    $<$<COMPILE_LANGUAGE:CXX>:-ffunction-sections>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-builtin>
    $<$<COMPILE_LANGUAGE:CXX>:-fPIC>
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

set(CMAKE_CXX_LINK_EXECUTABLE
    "ld -m elf_x86_64 -T ${CMAKE_SOURCE_DIR}/linker/kernel.ld <OBJECTS> -o <TARGET>")

# ESP staging
set(ESP_DIR ${CMAKE_BINARY_DIR}/esp)

add_custom_target(esp ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${ESP_DIR}/EFI/BOOT
    COMMAND ${CMAKE_COMMAND} -E copy ${BOOT_EFI} ${ESP_DIR}/EFI/BOOT/BOOTX64.EFI
    DEPENDS bootx64
    COMMENT "Staging ESP (build/esp)"
)
