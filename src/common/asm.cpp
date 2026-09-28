#include "common/asm.hpp"

namespace Lexvi {
    void halt() {
        ASM("cli; hlt");
    }
    
    unsigned char inb(unsigned short port) {
        unsigned char result;
        ASM("inb %1, %0" : "=a"(result) : "Nd"(port));
        return result;
    }
    
    void outb(unsigned short port, unsigned char value) {
        ASM("outb %0, %1" : : "a"(value), "Nd"(port));
    }
    
    unsigned short inw(unsigned short port) {
        unsigned short result;
        ASM("inw %1, %0" : "=a"(result) : "Nd"(port));
        return result;
    }
    
    void outw(unsigned short port, unsigned short value) {
        ASM("outw %0, %1" : : "a"(value), "Nd"(port));
    }
    
    unsigned int inl(unsigned short port) {
        unsigned int result;
        ASM("inl %1, %0" : "=a"(result) : "Nd"(port));
        return result;
    }
    
    void outl(unsigned short port, unsigned int value) {
        ASM("outl %0, %1" : : "a"(value), "Nd"(port));
    }
    
    void io_wait() {
        outb(0x80, 0x00);
    }
}
