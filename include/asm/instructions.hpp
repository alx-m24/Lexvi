#pragma once

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

inline void io_wait() {
    outb(0x80, 0x00);
}
