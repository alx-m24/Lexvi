#include "kernel/acpi/pci/pci.hpp"

#include "asm/instructions.hpp"
#include "kernel/acpi/mcfg.hpp"
#include "kernel/memory/memory-defs.hpp"

namespace kernel {
    uint64_t PCIConfigAddress::operator()(PCIConfigAddress::AccessType accessType) {
        switch (accessType) {
            case PCIConfigAddress::AccessType::MMIO: {
                    if (!cachedEntry) {
                        cachedEntry = mcfg_getEntry(bus);
                    }
                    return MMIO_TO_VIRT(
                       cachedEntry->base_address
                       + ((uint64_t)(bus - cachedEntry->start_bus) << 20)
                       + ((uint64_t)device << 15)
                       + ((uint64_t)function << 12)
                       + offset
                    );
            }
            case PCIConfigAddress::AccessType::LEGACY: {
                uint32_t val{};

                val |= (1u << 31); // enable bit
                val |= (bus << 16);
                val |= (device << 11);
                val |= (function << 8);
                val |= offset;

                return val;
            }
        };
    }   
    
    void PCIConfigAddress::setBus(uint8_t bus) {
        if (this->bus != bus) {
            this->bus = bus;
            cachedEntry = nullptr;
        }
    }

    void PCIConfigAddress::setDevice(uint8_t device) {
        this->device = device;
    }

    void PCIConfigAddress::setFunction(uint8_t function) {
        this->function = function;
    }

    void PCIConfigAddress::setOffset(uint8_t offset) {
        this->offset = offset;
    }

    uint8_t PCIConfigAddress::getBus() const {
        return bus;
    }

    uint8_t PCIConfigAddress::getDevice() const {
        return device;
    }

    uint8_t PCIConfigAddress::getFunction() const {
        return function;
    }

    uint8_t PCIConfigAddress::getOffset() const {
        return offset;
    }

    uint32_t pciConfigRead32(PCIConfigAddress address) {
        outl(0xCF8, address(PCIConfigAddress::AccessType::LEGACY));
        return inl(0xCFC);
    }

    void pciConfigWrite32(PCIConfigAddress address, uint32_t data) {
        outl(0xCF8, address(PCIConfigAddress::AccessType::LEGACY));
        outl(0xCFC, data);
    }

    // 8-bit Overloads
    uint8_t pciConfigRead8(PCIConfigAddress address, uint8_t lane) {
        KERNEL_ASSERT(lane < 4);
        // Clear the bottom 2 bits of the address for alignment in CF8
        outl(0xCF8, (address(PCIConfigAddress::AccessType::LEGACY) & ~3)); 
        // Add the byte lane directly to the data port
        return inb(0xCFC + lane); 
    }

    void pciConfigWrite8(PCIConfigAddress address, uint8_t lane, uint8_t data) {
        KERNEL_ASSERT(lane < 4);
        outl(0xCF8, (address(PCIConfigAddress::AccessType::LEGACY) & ~3));
        outb(0xCFC + lane, data);
    }

    // 16-bit Overloads
    uint16_t pciConfigRead16(PCIConfigAddress address, uint8_t lane) {
        KERNEL_ASSERT(lane < 4 && lane % 2 == 0);
        outl(0xCF8, (address(PCIConfigAddress::AccessType::LEGACY) & ~3)); // lane must be 0 or 2
        return inw(0xCFC + lane);
    }

    void pciConfigWrite16(PCIConfigAddress address, uint8_t lane, uint16_t data) {
        KERNEL_ASSERT(lane < 4 && lane % 2 == 0);
        outl(0xCF8, (address(PCIConfigAddress::AccessType::LEGACY) & ~3)); // lane must be 0 or 2
        outw(0xCFC + lane, data);
    }
}
