cmake_minimum_required(VERSION 3.25)
project(LexviOS CXX ASM_NASM)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)

set(GNUEFI_INC /usr/include/efi)
set(GNUEFI_LIB /usr/lib)

set(LEXVI_VENDOR   "" CACHE STRING "Target vendor (intel, amd)")
set(LEXVI_PLATFORM "" CACHE STRING "Target platform within vendor (e.g. 600)")
# Independent of CMAKE_BINARY_DIR so every preset writes to the same place
set(LEXVI_ESP_DIR "${CMAKE_SOURCE_DIR}/build/esp" CACHE PATH "Shared ESP staging directory")

if(LEXVI_VENDOR AND LEXVI_PLATFORM)
    set(LEXVI_BUILD_KERNEL ON)
elseif(LEXVI_VENDOR OR LEXVI_PLATFORM)
    message(FATAL_ERROR "Set BOTH -DLEXVI_VENDOR and -DLEXVI_PLATFORM, or neither (bootloader-only)")
else()
    set(LEXVI_BUILD_KERNEL OFF)
endif()

file(GLOB_RECURSE COMMON_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/common/*.c"
    "${CMAKE_SOURCE_DIR}/src/common/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/*.asm"
    "${CMAKE_SOURCE_DIR}/src/common/*.nasm")

# ---------------- Kernel (only when vendor+platform given) ----------------
if(LEXVI_BUILD_KERNEL)
    set(VENDOR_DIR          ${CMAKE_SOURCE_DIR}/src/vendor/${LEXVI_VENDOR})
    set(VENDOR_PLATFORM_DIR ${VENDOR_DIR}/${LEXVI_PLATFORM})

    if(NOT EXISTS ${VENDOR_PLATFORM_DIR})
        message(FATAL_ERROR "No sources at ${VENDOR_PLATFORM_DIR}")
    endif()

    file(GLOB_RECURSE VENDOR_SOURCES CONFIGURE_DEPENDS
        "${VENDOR_DIR}/common/*.c"
        "${VENDOR_DIR}/common/*.cpp"
        "${VENDOR_DIR}/common/*.asm"
        "${VENDOR_DIR}/common/*.nasm"
        "${VENDOR_PLATFORM_DIR}/*.c"
        "${VENDOR_PLATFORM_DIR}/*.cpp"
        "${VENDOR_PLATFORM_DIR}/*.asm"
        "${VENDOR_PLATFORM_DIR}/*.nasm")

    file(GLOB_RECURSE COMMON_KERNEL_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/src/kernel/*.c"
        "${CMAKE_SOURCE_DIR}/src/kernel/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/kernel/*.asm"
        "${CMAKE_SOURCE_DIR}/src/kernel/*.nasm")

    set(CMAKE_CXX_LINK_EXECUTABLE
        "ld -m elf_x86_64 -T ${CMAKE_SOURCE_DIR}/linker/kernel.ld <OBJECTS> <LINK_FLAGS> -o <TARGET>")

    add_executable(kernel
        ${COMMON_KERNEL_SOURCES}
        ${COMMON_SOURCES}
        ${VENDOR_SOURCES}
    )

    target_compile_definitions(kernel PRIVATE KERNEL)
    target_compile_options(kernel PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:-m64>
        $<$<COMPILE_LANGUAGE:CXX>:-ffreestanding>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-stack-protector>
        $<$<COMPILE_LANGUAGE:CXX>:-nostdlib>
        $<$<COMPILE_LANGUAGE:CXX>:-mno-red-zone>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    )

    target_include_directories(kernel PRIVATE
        include
        ${VENDOR_DIR}/common
        ${VENDOR_PLATFORM_DIR}
    )

    string(TOUPPER "${LEXVI_VENDOR}_${LEXVI_PLATFORM}" KERNEL_TAG)
    set_target_properties(kernel PROPERTIES
        LINKER_LANGUAGE CXX
        OUTPUT_NAME "KERNEL_${KERNEL_TAG}"
    )

    add_custom_command(TARGET kernel POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${LEXVI_ESP_DIR}/EFI/BOOT
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:kernel>
                ${LEXVI_ESP_DIR}/EFI/BOOT/KERNEL_${KERNEL_TAG}.BIN
        COMMENT "Staging KERNEL_${KERNEL_TAG}.BIN into shared ESP"
    )
endif()

# ---------------- Bootloader (only when vendor/platform NOT given) ----------------
if(NOT LEXVI_BUILD_KERNEL)
    file(GLOB_RECURSE BOOTLOADER_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/src/boot/*.c"
        "${CMAKE_SOURCE_DIR}/src/boot/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/boot/*.asm"
        "${CMAKE_SOURCE_DIR}/src/boot/*.nasm")

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
        DEPENDS bootloader_obj $<TARGET_OBJECTS:bootloader_obj>
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

    add_custom_target(esp ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory ${LEXVI_ESP_DIR}/EFI/BOOT
        COMMAND ${CMAKE_COMMAND} -E copy ${BOOT_EFI} ${LEXVI_ESP_DIR}/EFI/BOOT/BOOTX64.EFI
        DEPENDS bootx64
        COMMENT "Staging BOOTX64.EFI into shared ESP"
    )
endif()
