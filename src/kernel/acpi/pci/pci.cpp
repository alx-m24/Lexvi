#include "kernel/acpi/pci/pci.hpp"

#include "kernel/debug/gop.hpp"
#include "kernel/acpi/pci/statuscommand.hpp"
#include "kernel/acpi/mcfg.hpp"
#include "kernel/memory/memory-defs.hpp"

namespace kernel {
    uint64_t PCIConfigAddress::operator()() const {
        const MCFGEntry& entry = *mcfg_getEntry(bus);
    
        return MMIO_TO_VIRT(
            entry.base_address
            + ((uint64_t)(bus - entry.start_bus) << 20)
            + ((uint64_t)device << 15)
            + ((uint64_t)function << 12)
            + offset
        );
    }   

    inline uint32_t pciConfigRead32(const PCIConfigAddress address) {
        return *address;
    }

    inline void pciConfigWrite32(const PCIConfigAddress address, uint32_t data) {
        *address = data;
    }

    inline uint8_t pciConfigRead8(const PCIConfigAddress address, uint8_t lane) {
        KERNEL_ASSERT(lane < 4);
        // Clear the bottom 2 bits of the address for alignment in CF8
        return *reinterpret_cast<volatile uint8_t*>((address() & ~3) + lane); 
    }
    
    inline void pciConfigWrite8(const PCIConfigAddress address, uint8_t lane, uint8_t data) {
        KERNEL_ASSERT(lane < 4);
        *reinterpret_cast<volatile uint8_t*>((address() & ~3) + lane) = data;
    }
    
    inline uint16_t pciConfigRead16(const PCIConfigAddress address, uint8_t lane) {
        KERNEL_ASSERT(lane < 4 && lane % 2 == 0);
        return *reinterpret_cast<volatile uint16_t*>((address() & ~3) + lane); // lane must be 0 or 2
    }
    
    inline void pciConfigWrite16(const PCIConfigAddress address, uint8_t lane, uint16_t data) {
        KERNEL_ASSERT(lane < 4 && lane % 2 == 0);
        *reinterpret_cast<volatile uint16_t*>((address() & ~3) + lane) = data; // lane must be 0 or 2
    }

    bool pciTestWrite() {
        constexpr auto testAddr = kernel::PCIConfigAddress{0, 31, 0, 0x04}; // D31:F0, Command/Status dword

        uint32_t before = kernel::pciConfigRead32<testAddr>();
        KERNEL_PRINT("before: "); KERNEL_PRINTHEX(before); KERNEL_PRINT('\n');
        
        kernel::STATUSCOMMAND tmp = { before };   // reuse the existing Field definitions
        tmp.set<kernel::INTR_DISABLE>(!tmp.get<kernel::INTR_DISABLE>()); // flip just this one bit
        kernel::pciConfigWrite32<testAddr>(tmp());
        
        uint32_t after = kernel::pciConfigRead32<testAddr>();
        KERNEL_PRINT("after:  "); KERNEL_PRINTHEX(after); KERNEL_PRINT('\n');
        
        // restore original value regardless of outcome
        kernel::pciConfigWrite32<testAddr>(before);

        return tmp() == after;
    }
}
