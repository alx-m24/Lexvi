#include "kernel/acpi/pci/p2sb/p2sbc.hpp"

namespace kernel::P2SB {
#ifndef NDEBUG
    bool unhide_p2sb() {
        PCIConfigAddress p2sbc_address = P2SBC::getPCIConfigAddress();
    
        P2SBC p2sb{ *p2sbc_address };
        p2sb.set<SBILOCK>(0);
        p2sb.set<MASKLOCK>(0);
        p2sb.set<HIDE>(false);
    
        *p2sbc_address = p2sb();
    
        volatile uint32_t p2sb_after = *p2sbc_address; // force/verify the write landed
    
        PCIConfigAddress p2sb_id{ 0, 31, 1, 0x00 };
        return *p2sb_id != getInvalidPCIRegisterState<uint32_t>();
    }

    bool hide_p2sb() {
        PCIConfigAddress p2sbc_address = P2SBC::getPCIConfigAddress();
    
        P2SBC p2sb{ *p2sbc_address };
        p2sb.set<SBILOCK>(0);
        p2sb.set<MASKLOCK>(0);
        p2sb.set<HIDE>(true);
    
        *p2sbc_address = p2sb();
    
        volatile uint32_t p2sb_after = *p2sbc_address; // force/verify the write landed
    
        PCIConfigAddress p2sb_id{ 0, 31, 1, 0x00 };
        return *p2sb_id != getInvalidPCIRegisterState<uint32_t>();
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
