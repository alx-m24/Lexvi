#include "kernel/interrupt/interruptFrame.hpp"

uint64_t handle_syscall(const interrupt_frame_t& frame) {
    // rax = syscall number (by convention)
    // rdi, rsi, rdx = arguments
    
    uint64_t syscallNumber = frame.rax;
    uint64_t arguments[3] = { frame.rdi, frame.rsi, frame.rdx };

    return -1;
}
