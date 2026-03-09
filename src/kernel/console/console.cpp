#include "kernel/console/console.hpp"

namespace kernel {
    struct VGA_Character {
        char character;
        char colorAttribute;
    };
    
    VGA_Character* VGA_MEMORY = reinterpret_cast<VGA_Character*>(0xB8000);
    
    int cursorCol = 0;
    int cursorRow = 0;
    Color currentColorAttribute = Color::WHITE_ON_BLACK;
    
    constexpr unsigned int MAX_COLUMN = 80;
    constexpr unsigned int MAX_ROWS = 25;
    
    constexpr static unsigned int VGA_INDEX(const unsigned int col, const unsigned int row) {
        return col + row * MAX_COLUMN;
    }

    static char getColorAttribute(Color colorAttribute) {
        return static_cast<char>(colorAttribute);
    }
    
    void clearConsole() {
        for (int i = 0; i < MAX_COLUMN * MAX_ROWS; ++i) {
            VGA_MEMORY[i] = { .character = '\0', .colorAttribute = getColorAttribute(Color::FULL_BLACK) };
        }
    }

    void setColor(Color color) {
        currentColorAttribute = color;
    }

    ScopedColor::ScopedColor(const Color& color) {
        this->previousColor = currentColorAttribute;
        currentColorAttribute = color;
    }
    ScopedColor::~ScopedColor() {
        currentColorAttribute = this->previousColor;
    };
    
    void printf(char c) {
        if (cursorRow >= MAX_ROWS) { cursorRow = MAX_ROWS - 1; }
        if (cursorCol >= MAX_COLUMN) { cursorCol = 0; ++cursorRow; }
    
        if (c == '\n') {
            ++cursorRow;
            cursorCol = 0;
            return;
        }
    
        VGA_MEMORY[VGA_INDEX(cursorCol, cursorRow)] = { .character = c, .colorAttribute = getColorAttribute(currentColorAttribute) };
    
        ++cursorCol;
    }

    void printf(const char* str) {
        for (; *str != '\0'; ++str) {
            printf(*str);
        }
    }
   
    void printf(uint32_t n) {
        if (n == 0) {
            printf('0');
            return;
        }

        char buffer[10];
        int i = 0;
    
        while (n > 0) {
            buffer[i++] = (n % 10) + '0';
            n = n / 10;
        }
    
        while (i > 0) {
            printf(buffer[--i]);
        }
    }

    void printf(uint16_t n) {
        printf(static_cast<uint32_t>(n));
    }
    
    void printf(int n) {
        if (n < 0) {
            printf("-");
            n = -n;
        }

        printf(static_cast<unsigned int>(n));
    }

    void printfHex(uint64_t n) {
        char buffer[16]; // 16 hex digits for 64-bit numbers

        for(int i = 15; i >= 0; --i) {
            uint64_t digit = (n >> (i*4)) & 0xF;
            buffer[15-i] = digit < 10 ? '0'+digit : 'A'+(digit-10);
        }

        printf('0');
        printf('x');

        for(int i = 0; i < 16; ++i) {
            printf(buffer[i]);
        }
    }
}
