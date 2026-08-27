#include "kernel/acpi/fadt.hpp"

#include "kernel/utils/tables.hpp"
#include "kernel/debug/gop.hpp"

FADT* fadt = nullptr;

void loadFADT() {
    FADT* fadt = kernel::findTable<FADT>();
    KERNEL_ASSERT(fadt != nullptr);
    ::fadt = fadt;
}

void log_fadt() {
    KERNEL_ASSERT(fadt != nullptr);

    KERNEL_PRINT("FADT {\n");

    KERNEL_PRINT("\t- Header\n");
    KERNEL_PRINT("\t\t- FirmwareCtrl: ");
    KERNEL_PRINTHEX(fadt->FirmwareCtrl);
    KERNEL_PRINT("\n\t\t- Dsdt: ");
    KERNEL_PRINTHEX(fadt->Dsdt);
    KERNEL_PRINT("\n\t\t- Reserved: ", fadt->Reserved, "\n");

    KERNEL_PRINT("\t- Power Management\n");
    KERNEL_PRINT("\t\t- PreferredPowerManagementProfile: ", fadt->PreferredPowerManagementProfile, "\n");
    KERNEL_PRINT("\t\t- SCI_Interrupt: ");
    KERNEL_PRINTHEX(fadt->SCI_Interrupt);
    KERNEL_PRINT("\n\t\t- SMI_CommandPort: ");
    KERNEL_PRINTHEX(fadt->SMI_CommandPort);
    KERNEL_PRINT("\n\t\t- AcpiEnable: ");
    KERNEL_PRINTHEX(fadt->AcpiEnable);
    KERNEL_PRINT("\n\t\t- AcpiDisable: ");
    KERNEL_PRINTHEX(fadt->AcpiDisable);
    KERNEL_PRINT("\n\t\t- S4BIOS_REQ: ");
    KERNEL_PRINTHEX(fadt->S4BIOS_REQ);
    KERNEL_PRINT("\n\t\t- PSTATE_Control: ");
    KERNEL_PRINTHEX(fadt->PSTATE_Control);
    KERNEL_PRINT("\n\t\t- CStateControl: ", fadt->CStateControl, "\n");

    KERNEL_PRINT("\t- PM Blocks\n");
    KERNEL_PRINT("\t\t- PM1aEventBlock: ");
    KERNEL_PRINTHEX(fadt->PM1aEventBlock);
    KERNEL_PRINT("\n\t\t- PM1bEventBlock: ");
    KERNEL_PRINTHEX(fadt->PM1bEventBlock);
    KERNEL_PRINT("\n\t\t- PM1aControlBlock: ");
    KERNEL_PRINTHEX(fadt->PM1aControlBlock);
    KERNEL_PRINT("\n\t\t- PM1bControlBlock: ");
    KERNEL_PRINTHEX(fadt->PM1bControlBlock);
    KERNEL_PRINT("\n\t\t- PM2ControlBlock: ");
    KERNEL_PRINTHEX(fadt->PM2ControlBlock);
    KERNEL_PRINT("\n\t\t- PMTimerBlock: ");
    KERNEL_PRINTHEX(fadt->PMTimerBlock);
    KERNEL_PRINT("\n");

    KERNEL_PRINT("\t- PM Block Lengths\n");
    KERNEL_PRINT("\t\t- PM1EventLength: ", fadt->PM1EventLength, "\n");
    KERNEL_PRINT("\t\t- PM1ControlLength: ", fadt->PM1ControlLength, "\n");
    KERNEL_PRINT("\t\t- PM2ControlLength: ", fadt->PM2ControlLength, "\n");
    KERNEL_PRINT("\t\t- PMTimerLength: ", fadt->PMTimerLength, "\n");

    KERNEL_PRINT("\t- GPE\n");
    KERNEL_PRINT("\t\t- GPE0Block: ");
    KERNEL_PRINTHEX(fadt->GPE0Block);
    KERNEL_PRINT("\n\t\t- GPE1Block: ");
    KERNEL_PRINTHEX(fadt->GPE1Block);
    KERNEL_PRINT("\n\t\t- GPE0Length: ", fadt->GPE0Length, "\n");
    KERNEL_PRINT("\t\t- GPE1Length: ", fadt->GPE1Length, "\n");
    KERNEL_PRINT("\t\t- GPE1Base: ", fadt->GPE1Base, "\n");

    KERNEL_PRINT("\t- C-State / Timing\n");
    KERNEL_PRINT("\t\t- WorstC2Latency: ", fadt->WorstC2Latency, "\n");
    KERNEL_PRINT("\t\t- WorstC3Latency: ", fadt->WorstC3Latency, "\n");
    KERNEL_PRINT("\t\t- FlushSize: ", fadt->FlushSize, "\n");
    KERNEL_PRINT("\t\t- FlushStride: ", fadt->FlushStride, "\n");

    KERNEL_PRINT("\t- RTC / Alarm\n");
    KERNEL_PRINT("\t\t- DutyOffset: ", fadt->DutyOffset, "\n");
    KERNEL_PRINT("\t\t- DutyWidth: ", fadt->DutyWidth, "\n");
    KERNEL_PRINT("\t\t- DayAlarm: ", fadt->DayAlarm, "\n");
    KERNEL_PRINT("\t\t- MonthAlarm: ", fadt->MonthAlarm, "\n");
    KERNEL_PRINT("\t\t- Century: ", fadt->Century, "\n");

    KERNEL_PRINT("\t- Architecture / Flags\n");
    KERNEL_PRINT("\t\t- BootArchitectureFlags: ");
    KERNEL_PRINTHEX(fadt->BootArchitectureFlags);
    KERNEL_PRINT("\n\t\t- Reserved2: ", fadt->Reserved2, "\n");
    KERNEL_PRINT("\t\t- Flags: ");
    KERNEL_PRINTHEX(fadt->Flags);
    KERNEL_PRINT('\n');

    KERNEL_PRINT("\t- Reset\n");
    KERNEL_PRINT("\t\t- ResetReg: ");
    KERNEL_PRINTHEX(fadt->ResetReg.Address);
    KERNEL_PRINT("\n\t\t- ResetValue: ");
    KERNEL_PRINTHEX(fadt->ResetValue);
    KERNEL_PRINT('\n');

    KERNEL_PRINT("\t- Extended Addresses\n");
    KERNEL_PRINT("\t\t- X_FirmwareControl: ");
    KERNEL_PRINTHEX(fadt->X_FirmwareControl);
    KERNEL_PRINT("\n\t\t- X_Dsdt: ");
    KERNEL_PRINTHEX(fadt->X_Dsdt);
    KERNEL_PRINT('\n');

    KERNEL_PRINT("\t- Extended PM Blocks\n");
    KERNEL_PRINT("\t\t- X_PM1aEventBlock: ");
    KERNEL_PRINTHEX(fadt->X_PM1aEventBlock.Address);
    KERNEL_PRINT("\n\t\t- X_PM1bEventBlock: ");
    KERNEL_PRINTHEX(fadt->X_PM1bEventBlock.Address);
    KERNEL_PRINT("\n\t\t- X_PM1aControlBlock: ");
    KERNEL_PRINTHEX(fadt->X_PM1aControlBlock.Address);
    KERNEL_PRINT("\n\t\t- X_PM1bControlBlock: ");
    KERNEL_PRINTHEX(fadt->X_PM1bControlBlock.Address);
    KERNEL_PRINT("\n\t\t- X_PM2ControlBlock: ");
    KERNEL_PRINTHEX(fadt->X_PM2ControlBlock.Address);
    KERNEL_PRINT("\n\t\t- X_PMTimerBlock: ");
    KERNEL_PRINTHEX(fadt->X_PMTimerBlock.Address);
    KERNEL_PRINT("\n\t\t- X_GPE0Block: ");
    KERNEL_PRINTHEX(fadt->X_GPE0Block.Address);
    KERNEL_PRINT("\n\t\t- X_GPE1Block: ");
    KERNEL_PRINTHEX(fadt->X_GPE1Block.Address);
    KERNEL_PRINT('\n');

    KERNEL_PRINT("}\n");
}
