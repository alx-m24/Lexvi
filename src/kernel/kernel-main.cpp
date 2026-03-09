// Finally outside bootloader

#include "kernel/kernel.hpp"

extern "C" void kernel_main_cpp() {
    Kernel kernel;

    kernel.Run();
}
