#pragma once

#include "drawable.hpp"
#include "window.hpp"
#include <stdint.h>

namespace kernel {
    class Rectangle : public Drawable {
        private:
            uint16_t x, y;
            uint16_t width, height;
            uint16_t x2, y2;
            char fillChar;
            bool innerFilled;

        public:
            Rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, char fillChar = 0xDB, bool innerFilled = true)
                : Drawable(), x(x), y(y), width(width), height(height), x2(x + width), y2(y + height), fillChar(fillChar), innerFilled(innerFilled) {}

        public:
            void Draw(Window& window) const {
                for (uint16_t col = x; col < x2 && col < window.getWidth(); ++col) {
                    for (uint16_t row = y; row < y2 && row < window.getHeight(); ++row) {
                        bool isEdge = col == x || row == y || col == x2 - 1 || row == y2 - 1;
                        bool shouldFill = isEdge || innerFilled;
                        if (shouldFill) { 
                            window.DrawCharacter(col, row, fillChar);
                        }
                    }
                }
            }
    };
}
