#pragma once

#include "pci.hpp"
#include "kernel/register/register.hpp"

namespace kernel::ESPI {
    using DPE = Field<C_Bit, uint32_t, 31>;
    using SSE = Field<C_Bit, uint32_t, 30>;
    using RMA = Field<C_Bit, uint32_t, 29>;
    using RTA = Field<C_Bit, uint32_t, 28>;
    using STA = Field<C_Bit, uint32_t, 27>;
    using DTS = Field<const  uint8_t, uint32_t, 25, 26, true>;
    using DPD = Field<C_Bit, uint32_t, 24>;
    using FBC = Field<const  bool, uint32_t, 23>;
    using CLIST = Field<const  bool, uint32_t, 20>;
    using FBE = Field<const  bool, uint32_t, 9>;
    using SEE = Field<const  bool, uint32_t, 8>;
    using WCC = Field<const  bool, uint32_t, 7>;
    using PERE = Field<bool, uint32_t, 6>;
    using VGA_PSE = Field<const  bool, uint32_t, 5>;
    using MWIE = Field<const  bool, uint32_t, 4>;
    using SCE = Field<const  bool, uint32_t, 3>;
    using BME = Field<bool, uint32_t, 2>;
    using MSE = Field<const bool, uint32_t, 1>;
    using IOSE = Field<const bool, uint32_t, 0>;

    struct ESPI_STS_CMD : 
        public ReadWriteRegister<uint32_t,
                        getInvalidPCIRegisterState<uint32_t>(),
                        DPE,
                        SSE,
                        RMA,
                        RTA,
                        STA,
                        DTS,
                        DPD,
                        CLIST,
                        FBE,
                        SEE,
                        WCC,
                        PERE,
                        VGA_PSE,
                        MWIE,
                        SCE,
                        BME,
                        MSE,
                        IOSE> {
        ESPI_STS_CMD() = default;
        ESPI_STS_CMD(uint32_t val) : ReadWriteRegister(val) {}

        static constexpr PCIConfigAddress getPCIConfigAddress() {
            return { 0, 31, 0, 0x04 };
        }
    };
}
