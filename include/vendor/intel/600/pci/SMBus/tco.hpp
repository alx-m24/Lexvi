#pragma once

#include "kernel/acpi/pci/pci.hpp"
#include "kernel/register/register.hpp"

namespace kernel::SMBus {
    using TCO_BASE_ADDRESS  = Field<uint16_t,   uint32_t, 5, 15>;
    using IOS               = Field<const bool, uint32_t, 0>;

    struct TCO_BASE : 
        public ReadWriteRegister<uint32_t,
                                getInvalidPCIRegisterState<uint32_t>(),
                                TCO_BASE_ADDRESS,
                                IOS> {
        TCO_BASE() = default;
        TCO_BASE(uint32_t val) : ReadWriteRegister(val) {}

        static constexpr PCIConfigAddress getPCIConfigAddress() {
            return { 0, 31, 4, 0x50 };
        }
    };

    using TCO_BASE_ENABLED  = Field<bool, uint32_t, 8>;
    using TCO_BASE_LOCK     = Field<bool, uint32_t, 0>;

    struct TCO_CTL : 
        public ReadWriteRegister<uint32_t,
                                getInvalidPCIRegisterState<uint32_t>(),
                                TCO_BASE_ENABLED,
                                TCO_BASE_LOCK> {
        TCO_CTL() = default;
        TCO_CTL(uint32_t val) : ReadWriteRegister(val) {}

        static constexpr PCIConfigAddress getPCIConfigAddress() {
            return { 0, 31, 4, 0x54 };
        }
    };

    using TCO_LOCK          = Field<bool,       uint16_t, 12>;
    using TCO_TMR_HALT      = Field<bool,       uint16_t, 11>;
    using NMI2SMI_ENABLED   = Field<const bool, uint16_t,  9>;
    using NMI_NOW           = Field<const bool, uint16_t,  8>;

    // TCO_BASE_ADDRESS + 0x08
    struct TCO1_CNT : 
        public ReadWriteRegister<uint16_t,
                                getInvalidPCIRegisterState<uint16_t>(),
                                TCO_LOCK,
                                TCO_TMR_HALT,
                                NMI2SMI_ENABLED,
                                NMI_NOW> {
        TCO1_CNT() = default;
        TCO1_CNT(uint16_t val) : ReadWriteRegister(val) {}
    };

    bool disableTCO();
}
