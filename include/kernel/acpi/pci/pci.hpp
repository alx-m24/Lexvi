#pragma once

#include <cstdint>
#include <limits>

#include "kernel/error/error.hpp"
#include "kernel/register/register.hpp"

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

        // returns the virtual address of the uint32_t register pointed to by this address
        uint64_t operator()() const;

        volatile uint32_t& operator*() const {
            return *reinterpret_cast<volatile uint32_t*>((*this)());
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
        return *address;
    }

    template<PCIConfigAddress address>
    inline void pciConfigWrite32(uint32_t data) {
        *address = data;
    }

    template<PCIConfigAddress address>
    inline uint8_t pciConfigRead8(uint8_t lane) {
        KERNEL_ASSERT(lane < 4);
        // Clear the bottom 2 bits of the address for alignment in CF8
        return *reinterpret_cast<volatile uint8_t*>((address() & ~3) + lane); 
    }
    
    template<PCIConfigAddress address>
    inline void pciConfigWrite8(uint8_t lane, uint8_t data) {
        KERNEL_ASSERT(lane < 4);
        *reinterpret_cast<volatile uint8_t*>((address() & ~3) + lane) = data;
    }
    
    template<PCIConfigAddress address>
    inline uint16_t pciConfigRead16(uint8_t lane) {
        KERNEL_ASSERT(lane < 4 && lane % 2 == 0);
        return *reinterpret_cast<volatile uint16_t*>((address() & ~3) + lane); // lane must be 0 or 2
    }
    
    template<PCIConfigAddress address>
    inline void pciConfigWrite16(uint8_t lane, uint16_t data) {
        KERNEL_ASSERT(lane < 4 && lane % 2 == 0);
        *reinterpret_cast<volatile uint16_t*>((address() & ~3) + lane) = data; // lane must be 0 or 2
    }

    bool pciTestWrite();
}
