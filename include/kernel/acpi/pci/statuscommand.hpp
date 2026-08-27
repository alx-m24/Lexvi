#pragma once

#include "pci.hpp"
#include "kernel/register/register.hpp"

namespace kernel {
    using RMA           = Field<bool, uint32_t, 29>;
    using RTA           = Field<bool, uint32_t, 28>;
    using INTR_STATUS   = Field<bool, uint32_t, 19>;
    using INTR_DISABLE  = Field<bool, uint32_t, 10>;
    using SERR_ENABLE   = Field<bool, uint32_t, 8>;
    using BME           = Field<bool, uint32_t, 2>;
    // Memory Space Enable - This bit controls whether the host to PMC MMIO BAR is enabled or not
    using MSE = Field<bool, uint32_t, 1>; 

    struct STATUSCOMMAND 
        : public ReadWriteRegister<uint32_t, 
                                    getInvalidPCIRegisterState<uint32_t>(),
                                    RMA,
                                    RTA,
                                    INTR_STATUS,
                                    INTR_DISABLE,
                                    SERR_ENABLE,
                                    BME,
                                    MSE> {
        STATUSCOMMAND() = default;
        constexpr STATUSCOMMAND(uint32_t val) : ReadWriteRegister(val) {}
    
        static constexpr PCIConfigAddress getPCIConfigAddress() {
            return { 0, 31, 2, 0x04 };
        }
    };
}
