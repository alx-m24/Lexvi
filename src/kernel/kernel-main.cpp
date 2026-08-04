#include "kernel/kernel.hpp"

static const char* John14_6 = "I am the way and the truth and the life. No one comes to the Father except through me";

extern "C" void kernel_main_cpp() {
    Kernel kernel;

    kernel.Run();
}
