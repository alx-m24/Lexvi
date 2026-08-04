#pragma once

#include "kernel/memory/memory-manager.hpp"

class Kernel {
    public:
        kernel::MemoryManager memoryManager{};

    public:
        Kernel() = default;

    private:
        void Init();
        
    public:
        void Run();
};
