#pragma once

#include "pci.hpp"
#include "kernel/register/register.hpp"

namespace kernel {
    using SBILOCK   = Field<bool, uint32_t, 31>;
    using MASKLOCK  = Field<bool, uint32_t, 17>;
    using HIDE      = Field<bool, uint32_t,  8>;

    struct P2SBC : public 
                   ReadWriteRegister<uint32_t,
                                    getInvalidPCIRegisterState<uint32_t>(),
                                    SBILOCK,
                                    MASKLOCK,
                                    HIDE> {
        P2SBC() = default;
        P2SBC(uint32_t val) : ReadWriteRegister(val) {}

        static constexpr PCIConfigAddress getPCIConfigAddress() {
            return { 0, 31, 1, 0xE0 };
        }
    };

    bool unhide_p2sb();                                            
}
