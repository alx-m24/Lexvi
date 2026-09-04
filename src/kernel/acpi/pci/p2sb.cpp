#include "kernel/acpi/pci/p2sb/p2sbc.hpp"

#include "kernel/debug/gop.hpp"

namespace kernel::P2SB {
#ifndef NDEBUG
    bool unhide_p2sb() {
        // pciConfigWrite8(P2SBC::getPCIConfigAddress(), 1, 0x00); // clears HIDE only, byte-lane write
        P2SBC p2sbc{}; 

        // Setup P2SBC
        p2sbc.set<SBILOCK>(0);
        p2sbc.set<MASKLOCK>(0);
        p2sbc.set<HIDE>(false);

        pciConfigWrite32(P2SBC::getPCIConfigAddress(), p2sbc());

        KERNEL_PRINT("P2SBC Address: { ", 
                P2SBC::getPCIConfigAddress().getBus(), ", ",
                P2SBC::getPCIConfigAddress().getDevice(), ", ",
                P2SBC::getPCIConfigAddress().getFunction(), ", ",
                P2SBC::getPCIConfigAddress().getOffset(), " }\n");

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
        // pciConfigWrite8(P2SBC::getPCIConfigAddress(), 1, 0x01); // sets HIDE only, byte-lane write
        P2SBC p2sbc{};

        // Setup P2SBC
        p2sbc.set<SBILOCK>(0);
        p2sbc.set<MASKLOCK>(0);
        p2sbc.set<HIDE>(true);

        pciConfigWrite32(P2SBC::getPCIConfigAddress(), p2sbc());

        P2SBC p2sbc_after = { pciConfigRead32(P2SBC::getPCIConfigAddress()) };

        KERNEL_PRINT("          - P2SBC after:  ");
        KERNEL_PRINTHEX(p2sbc_after());
        KERNEL_PRINT('\n');

        return (p2sbc_after.get<HIDE>() == true);
    }
#else
    bool unhide_p2sb() {
        pciConfigWrite8(P2SBC::getPCIConfigAddress(), 1, 0x00); // clears HIDE only, byte-lane write
        
        P2SBC p2sbc{};

        // Setup P2SBC
        p2sbc.set<SBILOCK>(0);
        p2sbc.set<MASKLOCK>(0);
        p2sbc.set<HIDE>(false);

        pciConfigWrite32(P2SBC::getPCIConfigAddress(), p2sbc());

        return true;
    }

    bool hide_p2sb() {
        // pciConfigWrite8(P2SBC::getPCIConfigAddress(), 1, 0x01); // sets HIDE only, byte-lane write

        P2SBC p2sbc{};

        // Setup P2SBC
        p2sbc.set<SBILOCK>(0);
        p2sbc.set<MASKLOCK>(0);
        p2sbc.set<HIDE>(true);

        pciConfigWrite32(P2SBC::getPCIConfigAddress(), p2sbc());

        return true;
    }
#endif
}
