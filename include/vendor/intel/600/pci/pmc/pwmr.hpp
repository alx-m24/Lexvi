// PWMR - Power Management Controller MMIO Space
#pragma once

#include "kernel/acpi/pci/pci.hpp"

#include "kernel/register/register.hpp"

namespace kernel::PMC {
    using BASEADDR      = Field<uint32_t, uint32_t, 13, 31>;
    using SIZEINDICATOR = Field<uint16_t, uint32_t,  4, 12>;
    using TYPE          = Field<uint8_t,  uint32_t,  1,  2>;
    using MESSAGE_SPACE = Field<bool,     uint32_t,  0>;

    struct PWRMBASE : 
        public ReadWriteRegister<uint32_t,
                                getInvalidPCIRegisterState<uint32_t>(),
                                BASEADDR,
                                SIZEINDICATOR,
                                TYPE,
                                MESSAGE_SPACE> {
        PWRMBASE() = default;
        constexpr PWRMBASE(uint32_t val) : ReadWriteRegister(val) {}
        
        static constexpr PCIConfigAddress getPCIConfigAddress() {
            return { 0, 31, 2, 0x10 };
        }
    };

}
