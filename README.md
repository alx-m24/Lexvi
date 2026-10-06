# Lexvi

> ⚠️ **Hobby Project:** Lexvi is a from-scratch operating system built as a hands-on exploration of low-level programming, computer architecture, and x86-64 systems. It is not intended to compete with Linux, Windows, or macOS. The goal is to understand what happens underneath an operating system by building as much of it as possible from the ground up.

![Early Test on Dell Inspiron 15](Demo.jpg)

*Early Lexvi test running on a Dell Inspiron 15*

## About

**Lexvi** is a bare-metal x86-64 operating system primarily written in **C++ and Assembly**.

The project is built without relying on an existing operating system environment at runtime. Instead, Lexvi communicates directly with firmware and hardware, handling tasks such as bootstrapping the kernel, discovering system memory, locating ACPI tables, configuring paging, and setting up the framebuffer.

The project is **open source**. Contributions, forks, experiments, and discussions are welcome.

### Current Focus

Lexvi currently focuses on the foundations required to bring up an operating system:

* UEFI boot
* x86-64 kernel initialization
* Physical memory management
* Virtual memory and page tables
* ACPI table discovery
* GOP framebuffer output
* FAT filesystem support
* CPU identification
* Custom linker scripts
* Freestanding C++ development
* Low-level Assembly integration
* QEMU and real hardware testing

---

## Architecture

Lexvi currently boots through **UEFI** rather than legacy BIOS.

The general boot flow is:

```text
UEFI Firmware
      │
      ▼
BOOTX64.EFI
      │
      ├── Retrieve UEFI memory map
      ├── Locate ACPI tables
      ├── Initialize framebuffer
      ├── Prepare kernel environment
      └── Build initial page tables
      │
      ▼
ExitBootServices()
      │
      ▼
Lexvi Kernel
      │
      ├── Memory Management
      ├── Virtual Memory
      ├── ACPI
      ├── CPU / Platform
      ├── Filesystem
      └── Hardware Interfaces
```

The bootloader passes the information discovered during firmware initialization to the kernel through a `KernelBootInfo` structure. This avoids relying on fixed physical addresses and gives the kernel an explicit description of the environment it is starting in.

---

## UEFI Bootloader

`BOOTX64.EFI` is a PE32+ UEFI application built using `gnu-efi` headers.

`gnu-efi` is used primarily for the required UEFI structures, types, and calling conventions. The boot process itself is implemented explicitly rather than relying on a higher-level boot framework.

### Key components

* **`src/boot-uefi/main.cpp`**
  UEFI entry point and bootloader implementation.

* **`elf_x86_64_efi.lds`**
  Custom linker script used when producing the bootloader.

* **`objcopy`**
  Converts the linked ELF bootloader into a PE32+ `.efi` executable.

* **`EFI_MEMORY_DESCRIPTOR`**
  Provides the UEFI memory map used during kernel initialization.

* **GOP framebuffer**
  Provides graphical framebuffer access instead of legacy VGA text mode.

* **UEFI `ConfigurationTable`**
  Used to locate the ACPI RSDP without relying on legacy BIOS memory scanning.

* **`KernelBootInfo`**
  Transfers boot-time information from the UEFI environment to the kernel.

The previous two-stage NASM BIOS bootloader has been retired from the active build and remains available in the `legacy` branch.

---

## Memory Management

Lexvi manually manages its memory layout and does not rely on an existing operating system to provide virtual memory.

Custom linker scripts define the placement of important sections and provide the kernel with explicit control over its address space.

This includes:

* `.text`
* `.rodata`
* `.data`
* `.bss`
* Kernel stack
* Bootloader sections
* Page table structures

Physical memory is obtained through UEFI's `AllocatePages` and `AllocatePool` interfaces during boot rather than assuming fixed scratch addresses.

This is a deliberate change from the earlier BIOS implementation, which relied on hardcoded addresses such as `0x7000`. Such assumptions are not reliable once the system moves to a modern UEFI boot environment.

---

## Building

### Requirements

Lexvi is currently developed and tested primarily on Linux and WSL.

Required tools include:

* `cmake`
* `g++` or `clang++`
* `nasm`
* `gnu-efi`
* `objcopy`
* `dd`
* `mtools`
* `dosfstools`
* `gdisk` / `sgdisk`
* `ovmf`

### OVMF

Lexvi uses OVMF to provide UEFI firmware when running under QEMU.

Use your distribution's packaged OVMF firmware so that the CODE and VARS images are matched.

> **Important:** Hand-rolled or zero-filled VARS files may fail firmware volume validation. Use a pre-built OVMF CODE/VARS pair provided by your distribution.

### Build

```bash
./scripts/build.sh
./scripts/publish.sh
```

`build.sh` compiles the bootloader and kernel.

`publish.sh` creates the final bootable disk image.

The process consists of two stages:

### 1. EFI System Partition

An **64 MB FAT32 ESP image** is created at:

```text
build/esp.img
```

The image contains:

```text
/EFI/BOOT/BOOTX64.EFI
/kernel.bin
```

`mtools` is used to populate the filesystem, so the image does not need to be mounted or modified with root privileges.

### 2. GPT Disk Image

A **128 MB GPT disk image** is then created:

```text
build/lexvi.img
```

The image contains a single `EF00` EFI System Partition.

The ESP image is written into the partition using `dd`. The partition offset is obtained directly from `sgdisk` rather than being hardcoded, allowing the layout to remain correct if the partition alignment changes.

The result is a real GPT-partitioned UEFI bootable disk image that can be used with QEMU or written to a USB drive.

---

## Running Lexvi

### QEMU

QEMU is the recommended way to test Lexvi during development.

```bash
qemu-system-x86_64 \
    -drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_CODE.fd \
    -drive if=pflash,format=raw,file=/usr/share/OVMF/OVMF_VARS.fd \
    -drive format=raw,file=build/lexvi.img \
    -debugcon file:debug.log \
    -global isa-debugcon.iobase=0x402
```

The `debugcon` output provides a debugging channel during the earliest stages of boot, before the kernel's normal console or other output mechanisms are available.

This is particularly useful for diagnosing failures during firmware interaction and early kernel initialization.

---

## Real Hardware

Because Lexvi now uses standard UEFI boot, it can also run on compatible x86-64 hardware.

To test on real hardware:

1. Build and publish the disk image.
2. Write the image to a USB drive.
3. Boot the machine using its UEFI boot menu.
4. Select the Lexvi EFI boot entry.

Lexvi has been tested on real hardware in addition to QEMU.

---

## Extending the Kernel

Lexvi uses **CMake** as its build system.

Adding a new kernel component generally involves:

1. Create the `.cpp` or `.asm` source file.
2. Place it in the appropriate source directory.
3. Add it to the relevant `CMakeLists.txt`.
4. Rebuild the project.

```bash
./scripts/build.sh
```

The goal is to keep the build system explicit while avoiding unnecessary complexity as the kernel grows.

---

## Project Structure

A simplified view of the project is:

```text
Lexvi/
├── src/
│   ├── boot-uefi/
│   │   └── main.cpp
│   └── kernel/
├── scripts/
│   ├── build.sh
│   └── publish.sh
├── CMakeLists.txt
├── elf_x86_64_efi.lds
└── ...
```

The exact structure is evolving alongside the kernel.

---

## Development

Lexvi is actively developed as a learning and experimentation project.

The project intentionally favors understanding over abstraction. When possible, components are implemented directly so that their interaction with the underlying hardware and firmware remains visible.

Areas of ongoing development include:

* Improving memory management
* Expanding hardware support
* Developing kernel subsystems
* Improving filesystem support
* Expanding hardware testing
* Building toward a more complete kernel environment

---

## License

Lexvi is open source.

See the repository for the current license and contribution information.
