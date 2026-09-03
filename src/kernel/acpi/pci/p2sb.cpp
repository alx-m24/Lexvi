#include "kernel/acpi/pci/p2sb.hpp"

#include "kernel/debug/gop.hpp"

namespace kernel {
#ifndef NDEBUG
    bool unhide_p2sb() {
        pciConfigWrite8(P2SBC::getPCIConfigAddress(), 1, 0x00); // clears HIDE only, byte-lane write

        P2SBC p2sbc_after = { pciConfigRead32(P2SBC::getPCIConfigAddress()) };

        KERNEL_PRINT("          - P2SBC after:  ");
        KERNEL_PRINTHEX(p2sbc_after());
        KERNEL_PRINT('\n');

        uint32_t p2sb_id = pciConfigRead32(PCIConfigAddress(0, 31, 1, 0x00));

        KERNEL_PRINT("          - P2SB ID:      ");
        KERNEL_PRINTHEX(p2sb_id);
        KERNEL_PRINT('\n');

        // return (p2sbc_after.get<HIDE>() == false);
        return (p2sb_id != getInvalidPCIRegisterState<uint32_t>());
    }

    bool hide_p2sb() {
        pciConfigWrite8(P2SBC::getPCIConfigAddress(), 1, 0x01); // sets HIDE only, byte-lane write

        P2SBC p2sbc_after = { pciConfigRead32(P2SBC::getPCIConfigAddress()) };

        KERNEL_PRINT("          - P2SBC after:  ");
        KERNEL_PRINTHEX(p2sbc_after());
        KERNEL_PRINT('\n');

        return (p2sbc_after.get<HIDE>() == true);
    }
#else
    bool unhide_p2sb() {
        pciConfigWrite8(P2SBC::getPCIConfigAddress(), 1, 0x00); // clears HIDE only, byte-lane write

        return true;
    }

    bool hide_p2sb() {
        pciConfigWrite8(P2SBC::getPCIConfigAddress(), 1, 0x01); // sets HIDE only, byte-lane write
        
        return true;
    }
#endif
}
