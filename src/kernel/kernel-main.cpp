#include "kernel/kernel.hpp"

#include "kernel/kernel-config.hpp"

#include <cstdint>

static const char* John14_6 = "I am the way and the truth and the life. No one comes to the Father except through me";

extern "C" void kernel_main_cpp(uint64_t memory_map_address) {
    MEMORY_MAP_ADDRESS = memory_map_address;

    Kernel kernel;

    kernel.Run();
}
