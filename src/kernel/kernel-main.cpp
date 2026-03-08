// Finally outside bootloader

#include "kernel/io/console.hpp"

class Parent {
    public:
        virtual void OutputIdentity() {
            kernel::printf("This is the parent\n");
        }
};

class Derived : public Parent {
    void OutputIdentity() override {
        kernel::printf("This is the child\n");
    }
};

void PrintText(Parent* anyDerived) {
    anyDerived->OutputIdentity(); 
}

extern "C" void kernel_main() {
    kernel::clearConsole();
    kernel::printf("Welcome to the main kernel\n");

    Parent parent {};
    Derived derived {};

    PrintText(&parent);
    PrintText(&derived);

    while (true); 
}
