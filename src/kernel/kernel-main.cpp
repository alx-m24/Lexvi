// Finally outside bootloader

#include "kernel/io/console.hpp"
#include "kernel/error/error.hpp"

extern "C" void kernel_main() {
    kernel::clearConsole();
    kernel::printf("Hi from kernel_main\n");

    kernel::printf(-895, '\n');

    kernel::printf("Hi i am testing variadics: ", 10, 'c', '\n', "Test over\n");

    KERNEL_PANIC("this is only a test panic"); 

    while (true); 
}
