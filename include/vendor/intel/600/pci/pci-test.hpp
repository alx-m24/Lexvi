#pragma once

#include <tuple>

#include "pci.hpp"
#include "spi/vendor_deviceID.hpp"
#include "spi/status_command.hpp"
#include "pmc/statuscommand.hpp"

#include "kernel/debug/gop.hpp"
#include "kernel/utils/tests.hpp"

namespace kernel::tests {
    template<PCIRegister_T Reg, Field_T F>
    requires std::same_as<typename F::TYPE, bool> && isReadable<Reg, F> && isWritable<Reg, F>
    KERNEL_TEST_FUNC(
        pciWriteTest_Legacy,
        bool,
        (), {
            uint32_t before = pciConfigRead32(Reg::getPCIConfigAddress());

            KERNEL_PRINT("          - before: "); KERNEL_PRINTHEX(before); KERNEL_PRINT('\n');

            Reg tmp = { before };
            tmp.template set<F>(!tmp.template get<F>()); // flip just this one bit
            kernel::pciConfigWrite32(Reg::getPCIConfigAddress(), tmp());
            
            uint32_t after = kernel::pciConfigRead32(Reg::getPCIConfigAddress());
            KERNEL_PRINT("          - after:  "); KERNEL_PRINTHEX(after); KERNEL_PRINT('\n');
            
            // restore original value regardless of outcome
            kernel::pciConfigWrite32(Reg::getPCIConfigAddress(), before);

            return tmp() == after;
        }
    )

    template<PCIRegister_T Reg, Field_T F>
    requires std::same_as<typename F::TYPE, bool> && isReadable<Reg, F> && isWritable<Reg, F>
    KERNEL_TEST_FUNC(
        pciWriteTest_MMIO,
        bool,
        (), {
            PCIConfigAddress testAddr = Reg::getPCIConfigAddress();

            uint32_t before = *testAddr;
            KERNEL_PRINT("      - before: "); KERNEL_PRINTHEX(before); KERNEL_PRINT('\n');
            
            Reg tmp = { before };   // reuse the existing Field definitions
            tmp.template set<F>(!tmp.template get<F>()); // flip just this one bit
            *testAddr = tmp();
            
            uint32_t after = *testAddr;
            KERNEL_PRINT("      - after:  "); KERNEL_PRINTHEX(after); KERNEL_PRINT('\n');
            
            // restore original value regardless of outcome
            *testAddr = before;

            return tmp() == after;
        }
    )

    template<typename Reg, typename Field>
    struct PCIWriteTestCase {
        using Register = Reg;
        using FieldType = Field;
    };

    using STATUSCOMMAND_INTR_DISABLE = PCIWriteTestCase<PMC::STATUSCOMMAND, PMC::INTR_DISABLE>;
    using ESPI_STATUSCOMMAND_BME = PCIWriteTestCase<SPI::STATUS_COMMAND, SPI::BME>;

    using PCI_Write_TestCases = std::tuple<STATUSCOMMAND_INTR_DISABLE, ESPI_STATUSCOMMAND_BME>;

    template<typename TestCase>
    inline bool runPCIWriteTest_Legacy() {
        using Reg = typename TestCase::Register;
        using Field = typename TestCase::FieldType;
    
        return pciWriteTest_Legacy<Reg, Field>();
    }

    template<typename TestCase>
    inline bool runPCIWriteTest_MMIO() {
        using Reg = typename TestCase::Register;
        using Field = typename TestCase::FieldType;
    
        return pciWriteTest_MMIO<Reg, Field>();
    }

    KERNEL_TEST_FUNC(
        pciWriteTestRoutine,
        void,
        (), {

            bool successFullWrite = false;
            PCI_Write_TestCases testCases{};

            std::apply([&](auto&&... args) {
                ((successFullWrite |= runPCIWriteTest_Legacy<
                    std::remove_cvref_t<decltype(args)>
                >()), ...);

            }, testCases);

            if (successFullWrite) {
                KERNEL_PRINT("\t\t=== Legacy write SUCCESSFULL ===\n\n");
                successFullWrite = false;
            }
            else {
                KERNEL_PRINT("\t\t=== Legacy write FAILED ===\n\n");
            }

            std::apply([&](auto&&... args) {
                ((successFullWrite |= runPCIWriteTest_MMIO<
                    std::remove_cvref_t<decltype(args)>
                >()), ...);

            }, testCases);

            if (successFullWrite) {
                KERNEL_PRINT("\t\t=== MMIO write SUCCESSFULL ===\n\n");
                successFullWrite = false;
            }
            else {
                KERNEL_PRINT("\t\t=== MMIO write FAILED ===\n\n");
            }
        }
    )

    KERNEL_TEST_FUNC(
        testPCIRead,
        void,
        (), {
            SPI::VendorID vendorID{};
            SPI::DeviceID deviceID{};

            KERNEL_PRINT("\t\t- Getting Device ID LEGACY\n");
            bool successFullRead = getVendorDeviceID(PCIConfigAddress::AccessType::LEGACY, &vendorID, &deviceID);

            KERNEL_PRINT("\t\t\t- Vendor: ", vendorID.getVendorName(), "\n");
            KERNEL_PRINT("\t\t\t- Device: ");
            KERNEL_PRINTHEX(deviceID.getDeviceID());
            KERNEL_PRINT('\n');

            if (successFullRead){
                KERNEL_PRINT("\t\t=== Legacy read SUCCESSFULL ===\n\n");
            }
            else {
                KERNEL_PRINT("\t\t=== Legacy read FAILED ===\n\n");
            }

            KERNEL_PRINT("\t\t- Getting Device ID MMIO\n");
            successFullRead = getVendorDeviceID(PCIConfigAddress::AccessType::MMIO, &vendorID, &deviceID);

            KERNEL_PRINT("\t\t\t- Vendor: ", vendorID.getVendorName(), "\n");
            KERNEL_PRINT("\t\t\t- Device: ");
            KERNEL_PRINTHEX(deviceID.getDeviceID());
            KERNEL_PRINT('\n');

            if (successFullRead){
                KERNEL_PRINT("\t\t=== MMIO read SUCCESSFULL ===\n\n");
            }
            else {
                KERNEL_PRINT("\t\t=== MMIO read FAILED ===\n\n");
            }
        }
    )
}
