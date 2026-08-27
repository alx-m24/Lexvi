#include "kernel/acpi/pci/pci.hpp"

namespace kernel {
    uint32_t pciConfigRead32(const PCIConfigAddress address) {
        outl(0xCF8, address());
        return inl(0xCFC);
    }

    void pciConfigWrite32(const PCIConfigAddress address, uint32_t data) {
        outl(0xCF8, address());
        outl(0xCFC, data);
    }

    // 8-bit Overloads
    uint8_t pciConfigRead8(const PCIConfigAddress address, uint8_t lane) {
        KERNEL_ASSERT(lane < 4);
        // Clear the bottom 2 bits of the address for alignment in CF8
        outl(0xCF8, (address() & ~3)); 
        // Add the byte lane directly to the data port
        return inb(0xCFC + lane); 
    }
    
    void pciConfigWrite8(const PCIConfigAddress address, uint8_t lane, uint8_t data) {
        KERNEL_ASSERT(lane < 4);
        outl(0xCF8, (address() & ~3));
        outb(0xCFC + lane, data);
    }
    
    // 16-bit Overloads
    uint16_t pciConfigRead16(const PCIConfigAddress address, uint8_t lane) {
        KERNEL_ASSERT(lane < 4 && lane % 2 == 0);
        outl(0xCF8, (address() & ~3)); // lane must be 0 or 2
        return inw(0xCFC + lane);
    }
    
    void pciConfigWrite16(const PCIConfigAddress address, uint8_t lane, uint16_t data) {
        KERNEL_ASSERT(lane < 4 && lane % 2 == 0);
        outl(0xCF8, (address() & ~3)); // lane must be 0 or 2
        outw(0xCFC + lane, data);
    }
}
