#include "kernel/console/tui/window.hpp"

#include "kernel/console/fonts/font8*8_basic.h"
#include "kernel/console/console.hpp"
#include "kernel/error/error.hpp"

#include "asm/instructions.hpp"

static uint8_t reverse_bits(uint8_t b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

static void load_8x8_font() {
    outb(0x3C4, 0x02); outb(0x3C5, 0x04);
    outb(0x3C4, 0x04); outb(0x3C5, 0x07);
    outb(0x3CE, 0x04); outb(0x3CF, 0x02);
    outb(0x3CE, 0x05); outb(0x3CF, 0x00);
    outb(0x3CE, 0x06); outb(0x3CF, 0x04);

    uint8_t* font_plane = reinterpret_cast<uint8_t*>(0xA0000);
    for (int i = 0; i < 128; i++)
        for (int r = 0; r < 8; r++)
            font_plane[i * 32 + r] = reverse_bits(font8x8_basic[i][r]);

    outb(0x3C4, 0x02); outb(0x3C5, 0x03);
    outb(0x3C4, 0x04); outb(0x3C5, 0x03);
    outb(0x3CE, 0x04); outb(0x3CF, 0x00);
    outb(0x3CE, 0x05); outb(0x3CF, 0x10);
    outb(0x3CE, 0x06); outb(0x3CF, 0x0E);
}

namespace kernel {
    namespace Globals {
        constexpr uint16_t COLUMN_NUM = 80;
        constexpr uint16_t ROW_NUM = 50;
        VGA_Character* const VGA_MEMORY = reinterpret_cast<VGA_Character*>(0xB8000);
    }

    Window::Window() {
        // set vga mode to 80*50
        load_8x8_font();

        outb(0x3D4, 0x09);       // CRTC index: Max Scan Line
        outb(0x3D5, 0x07);       // 8 scan lines per char (8x8 font = 50 rows)
    }

    Window::Window(const Window* parent, uint16_t width, uint16_t height, uint16_t offsetX, uint16_t offsetY) 
        : m_parent(parent), m_width(width), m_height(height), m_offsetX(offsetX), m_offsetY(offsetY) 
    {
        KERNEL_ASSERT(parent != nullptr);
        KERNEL_ASSERT(offsetX + width  <= parent->m_width);
        KERNEL_ASSERT(offsetY + height <= parent->m_height);
    }

    uint64_t Window::getIndex(uint32_t col, uint32_t row) const {
        for (const Window* window = this; window != nullptr; window = window->m_parent) {
            col += window->m_offsetX;
            row += window->m_offsetY;
        }

        KERNEL_ASSERT(col < Globals::COLUMN_NUM && row < Globals::ROW_NUM);

        return col + row * Globals::COLUMN_NUM;
    }

    void Window::AdvanceCursor() {
        m_cursorX += 1;
        if (m_cursorX >= m_width) {
            m_cursorX = 0;
            m_cursorY += 1;
        }
        if (m_cursorY >= m_height) {
            m_cursorY = m_height - 1; // clamp to lowest line for now
        }
    }

    void Window::RetreatCursor() {
        if (m_cursorX > 0) {
            m_cursorX -= 1;
        }
        else if (m_cursorY > 0) {
            m_cursorY -= 1;
            m_cursorX = m_width - 1;
        }
    }

    void Window::clear() {
        for (uint16_t row = 0; row < m_height; ++row) {
            for (uint16_t col = 0; col < m_width; ++col) {
                Globals::VGA_MEMORY[getIndex(col, row)] = {};
            }
        }
        m_cursorX = m_cursorY = 0;
    }

    void Window::printf(char c) {
        if (c == '\0') return;

        if (c == '\b') {
            RetreatCursor();
            Globals::VGA_MEMORY[getIndex(m_cursorX, m_cursorY)] = {};
            return;
        }

        if (c == '\n') {
            m_cursorY += 1;
            if (m_cursorY >= m_height) m_cursorY = m_height - 1;
            m_cursorX = 0;
            return;
        }

        if (c == '\r') {
            m_cursorX = 0;
            return;
        }

        if (c == '\t') {
            uint16_t nextTab = (m_cursorX + 4) & ~3; // next multiple of 4
            while (m_cursorX < nextTab) {
                printf(' ');
            }
            return;
        }

        Globals::VGA_MEMORY[getIndex(m_cursorX, m_cursorY)] = VGA_Character{ .character = c, .colorAttribute = getColorAttribute(m_activeColor) };
        AdvanceCursor();
    }

    void Window::printf(const char* str) {
        for (; *str != '\0'; ++str) printf(*str);
    }

    void Window::printf(double d) {
        if (d != d) { printf("NaN"); return; }          // NaN check
        if (d < -1e308) { printf("-Inf"); return; }      // -Infinity
        if (d > 1e308)  { printf("Inf");  return; }      // +Infinity
        if (d > 1e19) { printf("(too large)"); return; } // avoiding numbers larger than UINT64_MAX
    
        if (d < 0) {
            printf('-');
            d = -d;
        }
    
        // print integer part
        printUnsigned(static_cast<uint64_t>(d));
        printf('.');
    
        // print fractional part to 6 decimal places
        d -= static_cast<uint64_t>(d);
        for (int i = 0; i < 6; ++i) {
            d *= 10;
            printf(static_cast<char>('0' + static_cast<uint64_t>(d) % 10));
        }
    }

    void Window::printfHex(uint64_t n) {
        char buffer[16]; // 16 hex digits for 64-bit numbers

        for(int i = 15; i >= 0; --i) {
            uint64_t digit = (n >> (i*4)) & 0xF;
            buffer[15-i] = digit < 10 ? '0'+digit : 'A'+(digit-10);
        }

        printf("0x");

        for(int i = 0; i < 16; ++i) {
            printf(buffer[i]);
        }
    }

    void Window::printSigned(int64_t n) {
        if (n < 0) {
            printf('-');
            printUnsigned(static_cast<uint64_t>(-(n + 1)) + 1);
        }
        else {
            printUnsigned(static_cast<uint64_t>(n));
        }
    }

    void Window::printUnsigned(uint64_t n) {
        if (n == 0) {
            printf('0');
            return;
        }

        char num[20] = {};
        int index = 0;

        while (n != 0) {
            num[index++] = '0' + (n % 10);
            n /= 10;
        }

        for (int i = index - 1; i >= 0; --i) {
            printf(num[i]);
        }
    }
}
