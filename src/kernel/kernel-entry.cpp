// Finally Some C++

#include "kernel/kernel-config.hpp"

struct VGA_Character {
    char character;
    char colorAttribute;
};

VGA_Character* VGA_MEMORY = reinterpret_cast<VGA_Character*>(0xB8000);

int cursorCol = 0;
int cursorRow = 0;

constexpr unsigned int MAX_COLUMN = 80;
constexpr unsigned int MAX_ROWS = 25;

constexpr unsigned int VGA_INDEX(const unsigned int col, const unsigned int row) {
    return col + row * MAX_COLUMN;
}

void kernel_printf(const char* str) {
    while (*str != '\0') {
        if (cursorRow >= MAX_ROWS) { cursorRow = MAX_ROWS - 1; }
        if (cursorCol >= MAX_COLUMN) { cursorCol = 0; ++cursorRow; }

        if (*str == '\n') {
            ++cursorRow;
            cursorCol = 0;
            ++str;
            continue;
        }

        VGA_MEMORY[VGA_INDEX(cursorCol, cursorRow)] = { .character = *str, .colorAttribute = 0x0F };

        ++cursorCol; ++str;
    }
}

void kernel_clearConsole() {
    for (int i = 0; i < MAX_COLUMN * MAX_ROWS; ++i) {
        VGA_MEMORY[i] = { .character = '\0', .colorAttribute = 0x0 };
    }
}

inline unsigned char inb(unsigned short port) {
    unsigned char result;
    asm volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

inline void outb(unsigned short port, unsigned char value) {
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

inline unsigned short inw(unsigned short port) {
    unsigned short result;
    asm volatile ("inw %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

void kernel_Delay400ns() {
    inb(0x1F7);
    inb(0x1F7);
    inb(0x1F7);
    inb(0x1F7);
}

bool kernel_isDriveBusy(unsigned char status) {
    return (status & 0b10000000);
}

bool kernel_isDriveReadyForTransfer(unsigned char status) {
    return status & 0b00001000;
}

void kernel_loadKernelMain() {
    outb(0x1F6, 0xE0);  // select master drive, LBA mode
    outb(0x3F6, 0x04);   // assert SRST bit in device control register
    outb(0x3F6, 0x00);   // clear SRST
    kernel_Delay400ns();

    kernel_printf("Waiting for drive...\n");
    while (kernel_isDriveBusy(inb(0x1F7)));
    kernel_printf("Drive ready\n");

    if (kernel_isDriveReadyForTransfer(inb(0x1F7))) kernel_printf("Ready to transfer\n");

    outb(0x1F2, static_cast<unsigned char>(KERNEL_MAIN_SECTORS));
    outb(0x1F3, static_cast<unsigned char>(KERNEL_MAIN_LBA));         // LBA bits 0-7
    outb(0x1F4, static_cast<unsigned char>(KERNEL_MAIN_LBA >> 8));    // LBA bits 8-15
    outb(0x1F5, static_cast<unsigned char>(KERNEL_MAIN_LBA >> 16));   // LBA bits 16-23
    outb(0x1F6, 0xE0 | static_cast<unsigned char>(KERNEL_MAIN_LBA >> 24)); // LBA bits 24-27
    outb(0x1F7, 0x20); // send read command

    kernel_Delay400ns();

    // wait for BSY to clear and DRQ to set
    while (kernel_isDriveBusy(inb(0x1F7)));
    if (!kernel_isDriveReadyForTransfer(inb(0x1F7))) {
        kernel_printf("Error: drive not ready for transfer\n");
        return;
    }

    kernel_printf("Loading kernel sectors into memory\n");

    // read sectors into memory
    unsigned short* dest = reinterpret_cast<unsigned short*>(KERNEL_MAIN_LOAD_ADDR);
    for (int sector = 0; sector < KERNEL_MAIN_SECTORS; ++sector) {
        // wait for DRQ per sector
        while (!kernel_isDriveReadyForTransfer(inb(0x1F7)));
        // read 256 words (512 bytes) per sector
        for (int word = 0; word < 256; ++word) {
            *dest++ = inw(0x1F0);
        }
    }
}

extern "C" void kernel_entry() {
    kernel_clearConsole(); 
    kernel_printf("Successfully loaded Kernel\n");

    kernel_loadKernelMain();

    kernel_printf("Jumping to kernelMain\n");

    // Jumping to main kernel
    typedef void (*KernelMain)();
    KernelMain kernelMain = reinterpret_cast<KernelMain>(KERNEL_MAIN_LOAD_ADDR);
    kernelMain();

    kernel_printf("Still in kernel entry?!\n");

    while (true);
}
