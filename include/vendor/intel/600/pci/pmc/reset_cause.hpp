#pragma once

#include "pwmr.hpp"

#include "kernel/register/register.hpp"
#include "kernel/acpi/pci/pci.hpp"
#include "kernel/acpi/fadt.hpp"

#include "kernel/memory/internals/vmm.hpp"
#include "kernel/memory/memory-defs.hpp"
#include "kernel/utils/math.hpp"
#include "kernel/debug/gop.hpp"

#include <iterator>
#include <cstddef>

namespace kernel::PMC {
    using PMC_RF_FUSA_ERR   = Field<C_Bit, uint32_t, 24>;
    using CPU_THRM_WDT      = Field<C_Bit, uint32_t, 16>;
    using SYSPWR_FLR        = Field<C_Bit, uint32_t, 12>;
    using PCHPWR_FLR        = Field<C_Bit, uint32_t, 11>;
    using PMC_FW            = Field<C_Bit, uint32_t, 10>;
    using CPU_THRM          = Field<C_Bit, uint32_t,  5>;
    using PCH_THRM          = Field<C_Bit, uint32_t,  3>;
    using PBO               = Field<C_Bit, uint32_t,  1>;

    struct GBLRST_CAUSE0 : 
        public ReadWriteRegister<uint32_t,
                                getInvalidPCIRegisterState<uint32_t>(),
                                PMC_RF_FUSA_ERR,
                                CPU_THRM_WDT,
                                SYSPWR_FLR,
                                PCHPWR_FLR,
                                PMC_FW,
                                CPU_THRM,
                                PCH_THRM,
                                PBO> {
        GBLRST_CAUSE0() = default;
        constexpr GBLRST_CAUSE0(uint32_t val) : ReadWriteRegister(val) {}

        static constexpr uint64_t getPWRMBaseOffset() {
            return 0x1924;
        }
    };

    using ESPI_TYPE8            = Field<C_Bit, uint32_t, 9>;
    using ESPI_TYPE7            = Field<C_Bit, uint32_t, 8>;
    using FW_GBLRST_SCRATCH5    = Field<C_Bit, uint32_t, 5>;
    using HSMB_MSG              = Field<C_Bit, uint32_t, 3>;
    using HOST_RST_PROM         = Field<C_Bit, uint32_t, 2>;
    using HOST_RESET_TIMEOUT    = Field<C_Bit, uint32_t, 0>;

    struct GBLRST_CAUSE1 : public ReadWriteRegister<uint32_t,
                                                    getInvalidPCIRegisterState<uint32_t>(),
                                                    ESPI_TYPE8,
                                                    ESPI_TYPE7,
                                                    FW_GBLRST_SCRATCH5,
                                                    HSMB_MSG,
                                                    HOST_RST_PROM,
                                                    HOST_RESET_TIMEOUT> {
        GBLRST_CAUSE1() = default;
        constexpr GBLRST_CAUSE1(uint32_t val) : ReadWriteRegister(val) {}

        static constexpr uint64_t getPWRMBaseOffset() {
            return 0x1928;
        }
    };

    using ESPI_HRWPC    = Field<bool, uint32_t, 17>;
    using ESPI_HRWOPC   = Field<bool, uint32_t, 16>;
    using HSMB_HRPC     = Field<bool, uint32_t, 13>;
    using HSMB_HR       = Field<bool, uint32_t, 12>;
    using MI_HRPD       = Field<bool, uint32_t, 10>;
    using MI_HRPC       = Field<bool, uint32_t,  9>;
    using MI_HR         = Field<bool, uint32_t,  8>;
    using TCO_WDT       = Field<bool, uint32_t,  6>;
    using SYSRST_ES     = Field<bool, uint32_t,  2>;
    using CF9_ES        = Field<bool, uint32_t,  1>;

    struct HPR_CAUSE0 : public ReadOnlyRegister<uint32_t,
                                                getInvalidPCIRegisterState<uint32_t>(),
                                                ESPI_HRWPC,
                                                ESPI_HRWOPC,
                                                HSMB_HRPC,
                                                HSMB_HR,
                                                MI_HRPD,
                                                MI_HRPC,
                                                MI_HR,
                                                TCO_WDT,
                                                SYSRST_ES,
                                                CF9_ES> {
        HPR_CAUSE0() = default;
        constexpr HPR_CAUSE0(uint32_t val) : ReadOnlyRegister(val) {}

        static constexpr uint64_t getPWRMBaseOffset() {
            return 0x192C;
        }
    };

    inline uint64_t getPwrmBaseFromFadt(const FADT* fadt_ptr) {
        if (!fadt_ptr) return 0;
    
        uint64_t pm_tmr_addr = 0;
        uint8_t space_id = 1; // Default to System I/O (1)
    
        // Check 64-bit Generic Address Structure (ACPI 2.0+)
        if (fadt_ptr->header.length >= offsetof(FADT, X_PMTimerBlock) + sizeof(GenericAddressStructure) 
            && fadt_ptr->X_PMTimerBlock.Address != 0) {
            pm_tmr_addr = fadt_ptr->X_PMTimerBlock.Address;
            space_id = fadt_ptr->X_PMTimerBlock.AddressSpace;
        } else {
            pm_tmr_addr = fadt_ptr->PMTimerBlock;
            space_id = 1; // 32-bit legacy PMTimerBlock is always System I/O
        }
    
        // PWRMBASE extraction only applies if ACPI PM timer is MMIO (SystemMemory = 0)
        if (pm_tmr_addr == 0 || space_id != 0) {
            return 0; // Not MMIO mapped
        }
    
        if (pm_tmr_addr < 0x18FC) {
            return 0; // Underflow guard
        }
    
        // Modern Intel PCH PMC MMIO Offset for ACPI PMTMR (ACPI_TMR_CTL) is 0x18FC
        uint64_t pwrm_base = (pm_tmr_addr - 0x18FC) & ~0xFFFULL; // Mask to 4KiB page boundary
    
        return pwrm_base;
    }

    inline void getResetCause(VMM& vmm) {
        PWRMBASE pwrmbase = { pciConfigRead32(PWRMBASE::getPCIConfigAddress()) };
        if (!pwrmbase) {
            KERNEL_PRINT("        - PWRMBASE register INVALID: Likely hidden\n");
            KERNEL_PRINT("          - Getting PWRMBASE from FADT\n");
            uint64_t pwrm_base = getPwrmBaseFromFadt(fadt);
            if (pwrm_base == 0) {
                KERNEL_PRINT("              - Failed to get PWRMBASE from FADT\n");
                return;
            }
            pwrmbase.set<BASEADDR>(pwrm_base);
        }

        constexpr auto GBLRST_CAUSE0_END =
            GBLRST_CAUSE0::getPWRMBaseOffset() + GBLRST_CAUSE0::SIZE;
        
        constexpr auto GBLRST_CAUSE1_END =
            GBLRST_CAUSE1::getPWRMBaseOffset() + GBLRST_CAUSE1::SIZE;
        
        constexpr auto HPR_CAUSE0_END =
            HPR_CAUSE0::getPWRMBaseOffset() + HPR_CAUSE0::SIZE;
        
        constexpr Bytes REQUIRED_SIZE = Bytes(
            max<GBLRST_CAUSE0_END, GBLRST_CAUSE1_END, HPR_CAUSE0_END>()
        );
        
        constexpr Bytes PAGE_SIZE = KiB(4).bytes();
        constexpr Bytes MMIO_SIZE = REQUIRED_SIZE.align_up(PAGE_SIZE);
        
        vmm.mapMMIO(MMIO_TO_VIRT(pwrmbase.get<BASEADDR>()), pwrmbase.get<BASEADDR>(), MMIO_SIZE);

        const auto pwrmBaseVirt = MMIO_TO_VIRT(pwrmbase.get<BASEADDR>());
        
        volatile uint32_t* GBLRST_CAUSE0_ptr =
            reinterpret_cast<volatile uint32_t*>(
                pwrmBaseVirt + GBLRST_CAUSE0::getPWRMBaseOffset());
        
        volatile uint32_t* GBLRST_CAUSE1_ptr =
            reinterpret_cast<volatile uint32_t*>(
                pwrmBaseVirt + GBLRST_CAUSE1::getPWRMBaseOffset());
        
        volatile uint32_t* HPR_CAUSE0_ptr =
            reinterpret_cast<volatile uint32_t*>(
                pwrmBaseVirt + HPR_CAUSE0::getPWRMBaseOffset());

        GBLRST_CAUSE0 gblrst_cause0 = { *GBLRST_CAUSE0_ptr };
        GBLRST_CAUSE1 gblrst_cause1 = { *GBLRST_CAUSE1_ptr };
        HPR_CAUSE0 hpr_cause0 = { *HPR_CAUSE0_ptr };

        static constexpr const char* GBLRST_CAUSE0_NAMES[] = {
            "PMC_RF_FUSA_ERR",
            "CPU_THRM_WDT",
            "SYSPWR_FLR",
            "PCHPWR_FLR",
            "PMC_FW",
            "CPU_THRM",
            "PCH_THRM",
            "PBO",
        };

        static constexpr const char* GBLRST_CAUSE1_NAMES[] {
            "ESPI_TYPE8", 
            "ESPI_TYPE7", 
            "FW_GBLRST_SCRATCH5", 
            "HSMB_MSG", 
            "HOST_RST_PROM", 
            "HOST_RESET_TIMEOUT"
        };

        static constexpr const char* HPR_CAUSE0_NAMES[] {
            "ESPI_HRWPC",
            "ESPI_HRWOPC",
            "HSMB_HRPC",
            "HSMB_HR",
            "MI_HRPD",
            "MI_HRPC",
            "MI_HR",
            "TCO_WDT",
            "SYSRST_ES",
            "CF9_ES",
        };

        KERNEL_PRINT("        - Reset causes: \n");

        C_Bit gblrst_cause0_bits[] = {
            gblrst_cause0.get<PMC_RF_FUSA_ERR>(),
            gblrst_cause0.get<CPU_THRM_WDT>(),
            gblrst_cause0.get<SYSPWR_FLR>(),
            gblrst_cause0.get<PCHPWR_FLR>(),
            gblrst_cause0.get<PMC_FW>(),
            gblrst_cause0.get<CPU_THRM>(),
            gblrst_cause0.get<PCH_THRM>(),
            gblrst_cause0.get<PBO>(),
        };

        for (uint32_t i{}; i < std::size(gblrst_cause0_bits); ++i) {
            if (gblrst_cause0_bits[i].getValue()) {
                KERNEL_PRINT("            - ", GBLRST_CAUSE0_NAMES[i], '\n'); 
            }
        }

        C_Bit gblrst_cause1_bits[] {
            gblrst_cause1.get<ESPI_TYPE8>(), 
            gblrst_cause1.get<ESPI_TYPE7>(), 
            gblrst_cause1.get<FW_GBLRST_SCRATCH5>(), 
            gblrst_cause1.get<HSMB_MSG>(), 
            gblrst_cause1.get<HOST_RST_PROM>(), 
            gblrst_cause1.get<HOST_RESET_TIMEOUT>()
        };

        for (uint32_t i{}; i < std::size(gblrst_cause1_bits); ++i) {
            if (gblrst_cause1_bits[i].getValue()) {
                KERNEL_PRINT("            - ", GBLRST_CAUSE1_NAMES[i], '\n'); 
            }
        }

        const bool hpr_cause0_bits[] {
            hpr_cause0.get<ESPI_HRWPC>(),
            hpr_cause0.get<ESPI_HRWOPC>(),
            hpr_cause0.get<HSMB_HRPC>(),
            hpr_cause0.get<HSMB_HR>(),
            hpr_cause0.get<MI_HRPD>(),
            hpr_cause0.get<MI_HRPC>(),
            hpr_cause0.get<MI_HR>(),
            hpr_cause0.get<TCO_WDT>(),
            hpr_cause0.get<SYSRST_ES>(),
            hpr_cause0.get<CF9_ES>()
        };

        for (uint32_t i{}; i < std::size(hpr_cause0_bits); ++i) {
            if (hpr_cause0_bits[i]) {
                KERNEL_PRINT("            - ", HPR_CAUSE0_NAMES[i], '\n'); 
            }
        }

    }
}
