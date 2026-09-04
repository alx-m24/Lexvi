#pragma once

#include <cstdint>
#include <concepts>
#include <limits>

#include "kernel/error/error.hpp"
#include "kernel/register/register.hpp"

struct MCFGEntry;

namespace kernel {
    template<RegType R>
    inline consteval R getInvalidPCIRegisterState() {
        return std::numeric_limits<R>::max();
    }

    class PCIConfigAddress {
        private:
        uint8_t bus{};
        uint8_t device{};
        uint8_t function{};
        uint8_t offset{};
        
        const MCFGEntry* cachedEntry = nullptr;

        public:
            PCIConfigAddress() = default;
            constexpr PCIConfigAddress(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) : bus(bus), device(device), function(function), offset(offset) { 
                KERNEL_ASSERT(device < 32);
                KERNEL_ASSERT(function < 8);
                KERNEL_ASSERT(offset % 4 == 0);
            }

            void setBus(uint8_t bus);
            void setDevice(uint8_t device);
            void setFunction(uint8_t function);
            void setOffset(uint8_t offset);

            uint8_t getBus() const;
            uint8_t getDevice() const;
            uint8_t getFunction() const;
            uint8_t getOffset() const;

        public:
            enum class AccessType : bool {
                MMIO = 0,
                LEGACY
            };

            // returns the virtual address of the uint32_t register pointed to by this address
            uint64_t operator()(AccessType accessType = AccessType::MMIO);

            volatile uint32_t& operator*() {
                return *reinterpret_cast<volatile uint32_t*>((*this)(AccessType::MMIO));
            }
    };

    template <typename T>
    concept HasGetPCIConfigAddress = requires(T a) {
        { a.getPCIConfigAddress() } -> std::same_as<PCIConfigAddress>;
    };

    template <typename T>
    concept PCIRegister_T = 
        HasGetPCIConfigAddress<T> && 
        is_derived_from_register_v<T>;

    uint32_t pciConfigRead32(PCIConfigAddress address);
    void pciConfigWrite32(PCIConfigAddress address, uint32_t data);

    uint8_t pciConfigRead8(PCIConfigAddress address, uint8_t lane);
    void pciConfigWrite8(PCIConfigAddress address, uint8_t lane, uint8_t data);

    uint16_t pciConfigRead16(PCIConfigAddress address, uint8_t lane);
    void pciConfigWrite16(PCIConfigAddress address, uint8_t lane, uint16_t data);
}
