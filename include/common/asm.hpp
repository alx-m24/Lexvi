#pragma once

// Collection of asm instructions

// #define ASM __asm__ volatile
#define ASM(...) __asm__ volatile (__VA_ARGS__)

namespace Lexvi {
    void halt();

    unsigned char inb(unsigned short port);
    
    void outb(unsigned short port, unsigned char value);
    
    unsigned short inw(unsigned short port);
    
    void outw(unsigned short port, unsigned short value);
    
    unsigned int inl(unsigned short port);
    
    void outl(unsigned short port, unsigned int value);
    
    void io_wait();
}
