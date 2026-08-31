#include "kernel/acpi/pci/p2sb.hpp"

#include "kernel/debug/gop.hpp"

namespace kernel {
    bool unhide_p2sb() {
        KERNEL_PRINT("          - P2SBC before: ");
        KERNEL_PRINTHEX(pciConfigRead32<P2SBC::getPCIConfigAddress()>());
        KERNEL_PRINT('\n');

        P2SBC p2sbc{0};

        p2sbc.set<SBILOCK>(0);
        p2sbc.set<MASKLOCK>(0);
        p2sbc.set<HIDE>(false);

        pciConfigWrite32<P2SBC::getPCIConfigAddress()>(p2sbc());

        P2SBC p2sbc_after = { pciConfigRead32<P2SBC::getPCIConfigAddress()>() };

        KERNEL_PRINT("          - P2SBC after:  ");
        KERNEL_PRINTHEX(p2sbc_after());
        KERNEL_PRINT('\n');

        return (p2sbc_after.get<HIDE>() == false);

    }
}
