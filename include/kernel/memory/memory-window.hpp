#pragma once

#include "kernel/console/tui/window.hpp"
#include "kernel/console/tui/outline.hpp"

namespace kernel {
    class MemoryWindow : public Window {
        public:
            MemoryWindow() = default;
            MemoryWindow(const Window* parent, uint16_t width, uint16_t height, uint16_t xOffset, uint16_t yOffset)
                : Window(parent, width, height, xOffset, yOffset) {
                    this->Draw(Outline(0, 0, width, height));
                    this->setPrintableArea(1, 1, width - 1, height - 1);
                }

        public:
            void Update() {

            }
    };
}
