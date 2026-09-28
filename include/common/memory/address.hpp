#pragma once

#include "common/memory/memory-unit.hpp"
#include "common/memory/memory-defs.hpp"

namespace Lexvi::Memory {
    enum class MemoryType {
        HHDM = 0,   // Physical address mapped through the HHDM
        LOWER_1MB,  // Physical address in the lower 1 MiB identity map
        MMIO,       // Physical address mapped through the MMIO window
        PHYSICAL    // Physical address with no mapping assumed
    };


    template<MemoryType Type>
    class Address;

    using PhysicalAddress = Address<MemoryType::PHYSICAL>;
    using HHDMAddress     = Address<MemoryType::HHDM>;
    using LowerAddress    = Address<MemoryType::LOWER_1MB>;
    using MMIOAddress     = Address<MemoryType::MMIO>;

    template<MemoryType Type>
    class AddressBase {
        protected:
            Bytes m_address{};

        public:
            constexpr explicit AddressBase(Bytes address) : m_address(address) {}

            static constexpr MemoryType TYPE = Type;

            constexpr AddressBase() = default;

            template<ByteRep_T Rep, ByteMultiple_T Scale>
            constexpr AddressBase(MemorySize<Rep, Scale> address)
                : m_address(address.bytes()) {}
            constexpr AddressBase(void* address)
                : m_address(reinterpret_cast<uint64_t>(address)) {}
            constexpr AddressBase(uint64_t address)
                : m_address(address) {}

            constexpr uint64_t getValue() const {
                return m_address.count();
            }

            constexpr Bytes getBytes() const {
                return m_address;
            }

            constexpr bool isNull() const {
                return m_address == Bytes(0);
            }

            constexpr operator bool() const {
                return !isNull();
            }
    };


    template<>
    class Address<MemoryType::PHYSICAL> : public AddressBase<MemoryType::PHYSICAL> {
        public:
            using AddressBase::AddressBase;

            constexpr HHDMAddress TO_HHDM() const;
            constexpr MMIOAddress TO_MMIO() const;
            constexpr LowerAddress TO_LOWER() const;
    };


    // HHDM virtual address
    template<>
    class Address<MemoryType::HHDM> : public AddressBase<MemoryType::HHDM> {
        public:
            using AddressBase::AddressBase;

            template<typename T = void>
            constexpr T* getPtr() const {
                return reinterpret_cast<T*>(getValue());
            }

            constexpr PhysicalAddress TO_PHYS() const;
    };


    template<>
    class Address<MemoryType::LOWER_1MB> : public AddressBase<MemoryType::LOWER_1MB> {
        public:
            using AddressBase::AddressBase;

            template<typename T = void>
            constexpr T* getPtr() const {
                return reinterpret_cast<T*>(getValue());
            }

            constexpr PhysicalAddress TO_PHYS() const;
    };


    template<>
    class Address<MemoryType::MMIO> : public AddressBase<MemoryType::MMIO> {
        public:
            using AddressBase::AddressBase;

            template<typename T = void>
            constexpr volatile T* getPtr() const {
                return reinterpret_cast<volatile T*>(getValue());
            }

            constexpr PhysicalAddress TO_PHYS() const;
    };


    constexpr HHDMAddress
    Address<MemoryType::PHYSICAL>::TO_HHDM() const {
        return HHDMAddress(m_address + HHDM_BASE);
    }


    constexpr MMIOAddress
    Address<MemoryType::PHYSICAL>::TO_MMIO() const {
        return MMIOAddress(m_address + MMIO_BASE);
    }


    constexpr LowerAddress
    Address<MemoryType::PHYSICAL>::TO_LOWER() const {
        return LowerAddress(m_address);
    }


    constexpr PhysicalAddress
    Address<MemoryType::HHDM>::TO_PHYS() const {
        return PhysicalAddress(m_address - HHDM_BASE);
    }


    constexpr PhysicalAddress
    Address<MemoryType::LOWER_1MB>::TO_PHYS() const {
        // Lower 1 MiB is identity mapped:
        return PhysicalAddress(m_address);
    }


    constexpr PhysicalAddress
    Address<MemoryType::MMIO>::TO_PHYS() const {
        return PhysicalAddress(m_address - MMIO_BASE);
    }

}
