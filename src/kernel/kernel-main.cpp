#include "boot/bootInfo.hpp"

#include "kernel/hal/init.hpp"
#include "kernel/hal/reset_cause.hpp"

#include "common/output/output-defs.hpp"

using namespace Lexvi::Kernel;
using namespace Lexvi::Kernel::hal::pwrm;

extern "C" void kernel_main_cpp(Lexvi::Boot::BootInfo* bootInfo) {
    LEXVI_PRINT("Welcome to Lexvi Kernel!!!\n");

    hal::init(); 

    ResetCause resetCause = getResetCause();
    if (resetCause == ResetCause::NONE) {
        LEXVI_PRINT("- No Reset\n"); 
    }

    while (true) { }
}
