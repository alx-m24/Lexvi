// Finally outside bootloader

#include "kernel/io/console.hpp"
#include "kernel/error/error.hpp"

extern "C" void kernel_main() {
    kernel::clearConsole();
    kernel::printf("Hi from kernel_main\n");

    {
        kernel::ScopedColor scopedColor(kernel::Color::CYAN_ON_BLACK);
        kernel::printf(-895, '\n');
    }

    kernel::printf("Hi i am testing variadics: ", 10, 'c', '\n', "Test over\n");

    {
        kernel::ScopedColor scopedColor(kernel::Color::RED_ON_BLACK);
        kernel::printf("Red test\n");
    }

    kernel::printfHex(15);

    int test = 0;
    KERNEL_ASSERT(test == 69);

    while (true); 
}
