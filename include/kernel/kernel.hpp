#pragma once

#include "kernel/console/tui/window.hpp"
#include "kernel/memory/memory-window.hpp"

class Kernel {
    public:
        static kernel::Window inputWindow ;
        
        kernel::Window mainWindow {};
        kernel::Window LeftSplit {};
        kernel::Window logWindow {};
        kernel::MemoryWindow memoryWindow {};

    public:
        Kernel() = default;

    private:
        void Init();
        
    public:
        void Run();

        static void Tick() {
            inputWindow.BlinkCursor();
        }
};
