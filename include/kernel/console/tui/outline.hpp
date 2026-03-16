#pragma once

#include "drawable.hpp"
#include "window.hpp"

namespace kernel {
    class Outline : public Drawable {
        private:
            uint16_t x, y;
            uint16_t width, height;

        public:
            Outline(uint16_t x, uint16_t y, uint16_t width, uint16_t height) : Drawable(), x(x), y(y), width(width), height(height) {}

        public:
            void Draw(Window& window) const override {
                // CP437 box-drawing characters
                constexpr char HORIZONTAL = 0xC4;  // ─
                constexpr char VERTICAL   = 0xB3;  // │
                constexpr char TOP_LEFT   = 0xDA;  // ┌
                constexpr char TOP_RIGHT  = 0xBF;  // ┐
                constexpr char BOT_LEFT   = 0xC0;  // └
                constexpr char BOT_RIGHT  = 0xD9;  // ┘
    
                // corners
                window.DrawCharacter(x,             y,            TOP_LEFT);
                window.DrawCharacter(x + width - 1, y,            TOP_RIGHT);
                window.DrawCharacter(x,             y + height - 1, BOT_LEFT);
                window.DrawCharacter(x + width - 1, y + height - 1, BOT_RIGHT);
    
                // top and bottom edges
                for (uint16_t col = x + 1; col < x + width - 1; ++col) {
                    window.DrawCharacter(col, y,            HORIZONTAL);
                    window.DrawCharacter(col, y + height - 1, HORIZONTAL);
                }
    
                // left and right edges
                for (uint16_t row = y + 1; row < y + height - 1; ++row) {
                    window.DrawCharacter(x,             row, VERTICAL);
                    window.DrawCharacter(x + width - 1, row, VERTICAL);
                }
            }
    };
}
