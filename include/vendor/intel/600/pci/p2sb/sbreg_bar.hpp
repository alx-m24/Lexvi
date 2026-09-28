#pragma once

#include "p2sbc.hpp"
#include "kernel/acpi/pci/pci.hpp"
#include "kernel/register/register.hpp"

namespace kernel::P2SB {
    using RBA = Field<uint32_t, uint32_t, 28, 31, false>;
    using HW202RB16MB = Field<const uint32_t, uint32_t, 4, 27, false>;
    using PREF = Field<const bool, uint32_t, 3>;
    using ATYPE = Field<const uint8_t, uint32_t, 1, 2, true>;
    using STYPE = Field<const bool, uint32_t, 0>;
    struct SBREG_BAR : public ReadWriteRegister<uint32_t,
                                                getInvalidPCIRegisterState<uint32_t>(),
                                                RBA,
                                                HW202RB16MB,
                                                PREF,
                                                ATYPE,
                                                STYPE> {
        SBREG_BAR() = default;
        SBREG_BAR(uint32_t val) : ReadWriteRegister(val) {}

        static constexpr PCIConfigAddress getPCIConfigAddress() {
            return { 0, 31, 1, 0x10 };
        }
    };

    using RBAH = Field<uint32_t, uint32_t, 0, 31, false>;
    struct SBREG_BARH : public ReadWriteRegister<uint32_t,
                                                getInvalidPCIRegisterState<uint32_t>(),
                                                RBAH> {
        SBREG_BARH() = default;
        SBREG_BARH(uint32_t val) : ReadWriteRegister(val) {}

        static constexpr PCIConfigAddress getPCIConfigAddress() {
            return { 0, 31, 1, 0x14 };
        }
    };

    inline uint64_t calculate_SBREG_BAR(const SBREG_BAR& sbreg_bar, const SBREG_BARH& sbreg_barh) {
        return static_cast<uint64_t>(sbreg_bar.get<RBA>())
             | (static_cast<uint64_t>(sbreg_barh.get<RBAH>()) << 32);
    }

    inline uint64_t get_SBREG_BAR() {
        ScopedP2SBCUnhide unhide{};

        SBREG_BAR sbregBar = *SBREG_BAR::getPCIConfigAddress();
        KERNEL_ASSERT(sbregBar == true);

        SBREG_BARH sbregBarh = *SBREG_BARH::getPCIConfigAddress();
        KERNEL_ASSERT(sbregBarh == true);

        return calculate_SBREG_BAR(sbregBar, sbregBarh);
    }
}
