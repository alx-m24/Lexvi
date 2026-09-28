#include "common/syscall/syscall.hpp"

namespace Lexvi::Interrupt {
    uint64_t handle_syscall(const Frame& frame) {
        // rax = syscall number (by convention)
        // rdi, rsi, rdx = arguments
        
        uint64_t syscallNumber = frame.rax;
        uint64_t arguments[3] = { frame.rdi, frame.rsi, frame.rdx };
    
        return -1;
    }
}
