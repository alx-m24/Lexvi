#include "boot/stages/stage1.hpp"

extern "C" {
    #include <efi/efi.h>
    #include <efi/efilib.h>
}

#include "boot/memory/alloc.hpp"
#include "boot/acpi/tables.hpp"
#include "boot/bootInfo.hpp"

#include "common/acpi/sdt.hpp"
#include "common/acpi/rsdp.hpp"
#include "common/acpi/fadt.hpp"
#include "common/acpi/hpet.hpp"
#include "common/acpi/mcfg.hpp"
#include "common/output/gop.hpp"
#include "common/types/service.hpp"

using namespace Lexvi::Memory;
using namespace Lexvi::Output;
using namespace Lexvi::Types;

namespace Lexvi::Boot::Stage1 {
    Result Run(BootInfo& bootInfo) {
        Result result{};

        // Init GOP/RSDP/SDT
        using CommonBootServices_T = ServiceList<GOP, Lexvi::ACPI::RSDP, Lexvi::ACPI::SDTHeader>;
        CommonBootServices_T commonBootServices{};
        result = commonBootServices.InitAll(); 
        if (result.IsError()) {
            return result;
        }

        bootInfo.GOP_Address = Memory::AllocPersistent<GOP_Data>();
        *reinterpret_cast<GOP_Data*>(bootInfo.GOP_Address.getValue()) = Lexvi::Output::gop;

        bootInfo.RSDP_Address = Memory::AllocPersistent<Lexvi::ACPI::RSDP>();
        *reinterpret_cast<Lexvi::ACPI::RSDP*>(bootInfo.RSDP_Address.getValue()) = Lexvi::ACPI::rsdp;

        bootInfo.SDTHeader_Address = Memory::AllocPersistent<Lexvi::ACPI::SDTHeader*>();
        *reinterpret_cast<Lexvi::ACPI::SDTHeader**>(bootInfo.SDTHeader_Address.getValue()) = Lexvi::ACPI::sdtHeader;
        
        bootInfo.FADT_Address = ACPI::findTable<Lexvi::ACPI::FADT>();
        bootInfo.HPET_Address = ACPI::findTable<Lexvi::ACPI::HPETTable>();
        bootInfo.MCFG_Address = ACPI::findTable<Lexvi::ACPI::MCFGTable>();

        return result;
    }
}
