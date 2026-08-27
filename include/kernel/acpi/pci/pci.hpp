#pragma once

#include <cstdint>
#include <limits>

#include "kernel/error/error.hpp"
#include "kernel/register/register.hpp"

#include "asm/instructions.hpp"

namespace kernel {
    template<RegType R>
    inline consteval R getInvalidPCIRegisterState() {
        return std::numeric_limits<R>::max();
    }

    struct PCIConfigAddress {
        uint8_t bus{};
        uint8_t device{};
        uint8_t function{};
        uint8_t offset{};

        PCIConfigAddress() = default;
        constexpr PCIConfigAddress(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) : bus(bus), device(device), function(function), offset(offset) { 
            KERNEL_ASSERT(device < 32);
            KERNEL_ASSERT(function < 8);
            KERNEL_ASSERT(offset % 4 == 0);
        }

        constexpr uint32_t operator()() const {
            uint32_t val{};

            val |= (1u << 31); // enable bit
            val |= (bus << 16);
            val |= (device << 11);
            val |= (function << 8);
            val |= offset;

            return val;
        }
    };

    uint32_t pciConfigRead32(const PCIConfigAddress address);
    void pciConfigWrite32(const PCIConfigAddress address, uint32_t data);

    uint8_t pciConfigRead8(const PCIConfigAddress address, uint8_t lane);
    void pciConfigWrite8(const PCIConfigAddress address, uint8_t lane, uint8_t data);
    
    uint16_t pciConfigRead16(const PCIConfigAddress address, uint8_t lane);
    void pciConfigWrite16(const PCIConfigAddress address, uint8_t lane, uint16_t data);

    template<PCIConfigAddress address>
    inline uint32_t pciConfigRead32() {
        outl(0xCF8, address());
        return inl(0xCFC);
    }

    template<PCIConfigAddress address>
    inline void pciConfigWrite32(uint32_t data) {
        outl(0xCF8, address());
        outl(0xCFC, data);
    }

    template<PCIConfigAddress address>
    inline uint8_t pciConfigRead8(uint8_t lane) {
        KERNEL_ASSERT(lane < 4);
        // Clear the bottom 2 bits of the address for alignment in CF8
        outl(0xCF8, (address() & ~3)); 
        // Add the byte lane directly to the data port
        return inb(0xCFC + lane); 
    }
    
    template<PCIConfigAddress address>
    inline void pciConfigWrite8(uint8_t lane, uint8_t data) {
        KERNEL_ASSERT(lane < 4);
        outl(0xCF8, (address() & ~3));
        outb(0xCFC + lane, data);
    }
    
    template<PCIConfigAddress address>
    inline uint16_t pciConfigRead16(uint8_t lane) {
        KERNEL_ASSERT(lane < 4 && lane % 2 == 0);
        outl(0xCF8, (address() & ~3)); // lane must be 0 or 2
        return inw(0xCFC + lane);
    }
    
    template<PCIConfigAddress address>
    inline void pciConfigWrite16(uint8_t lane, uint16_t data) {
        KERNEL_ASSERT(lane < 4 && lane % 2 == 0);
        outl(0xCF8, (address() & ~3)); // lane must be 0 or 2
        outw(0xCFC + lane, data);
    }
}
