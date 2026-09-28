#include "kernel/acpi/pci/SMBus/tco.hpp"

#include "kernel/debug/gop.hpp"
#include "asm/instructions.hpp"

namespace kernel::SMBus {
    bool disableTCO() {
        KERNEL_PRINT("   - Disabling TCO\n");

        TCO_BASE tcoBase = { kernel::pciConfigRead32(TCO_BASE::getPCIConfigAddress()) };
        if (!tcoBase) {
            KERNEL_PRINT("      - Invalid TCO_BASE: Likely Hidden\n");
            return false;
        }

        KERNEL_PRINT("      - TCO_BASE_ADDRESS: ");
        KERNEL_PRINTHEX(tcoBase.get<TCO_BASE_ADDRESS>());
        KERNEL_PRINT('\n');

        TCO_CTL tcoCTL = { kernel::pciConfigRead32(TCO_CTL::getPCIConfigAddress()) };
        if (!tcoCTL) {
            KERNEL_PRINT("      - Invalid TCO_CTL: Likely Hidden\n");
            return false;
        }

        if (tcoCTL.get<TCO_BASE_ENABLED>() == false) {
            KERNEL_PRINT("      - TCO Control is Disabled\n");
            return false;
        }

        TCO1_CNT tco1_cnt = inw(tcoBase.get<TCO_BASE_ADDRESS>() + 0x08);
        if (!tco1_cnt) {
            KERNEL_PRINT("      - Invalid TCO1_CNT: Likely Hidden\n");
            return false;
        }
        if (tco1_cnt.get<TCO_TMR_HALT>()) {
            KERNEL_PRINT("      - TCO already halted\n");
            return true;
        }
        
        KERNEL_PRINT("      - TCO1_CNT before: ");
        KERNEL_PRINTHEX(inw(tcoBase.get<TCO_BASE_ADDRESS>() + 0x08));
        KERNEL_PRINT('\n');

        tco1_cnt.set<TCO_TMR_HALT>(true);
        outw(tcoBase.get<TCO_BASE_ADDRESS>() + 0x08, tco1_cnt());

        uint16_t tco1_after = inw(tcoBase.get<TCO_BASE_ADDRESS>() + 0x08);
        
        KERNEL_PRINT("      - TCO1_CNT after: ");
        KERNEL_PRINTHEX(tco1_after);
        KERNEL_PRINT('\n');
        
        return TCO1_CNT(tco1_after).get<TCO_TMR_HALT>();
    }
}
