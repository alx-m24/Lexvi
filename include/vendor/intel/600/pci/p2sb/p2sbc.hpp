#pragma once

#include "kernel/acpi/pci/pci.hpp"
#include "kernel/register/register.hpp"

namespace kernel::P2SB {
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
    bool hide_p2sb();

    class ScopedP2SBCUnhide {
        public:
            ScopedP2SBCUnhide() { 
                bool unhideSuccess = unhide_p2sb();
                // KERNEL_ASSERT(unhideSuccess == true);
            }
            ~ScopedP2SBCUnhide() { 
                bool hideSuccess = hide_p2sb();
                // KERNEL_ASSERT(hideSuccess == true);
            }

            ScopedP2SBCUnhide(const ScopedP2SBCUnhide&) = delete;
            ScopedP2SBCUnhide& operator=(const ScopedP2SBCUnhide&) = delete;

            ScopedP2SBCUnhide(ScopedP2SBCUnhide&&) = delete;
            ScopedP2SBCUnhide& operator=(ScopedP2SBCUnhide&&) = delete;
    };
}
