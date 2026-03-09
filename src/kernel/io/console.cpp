#include "kernel/io/console.hpp"

namespace kernel {
    struct VGA_Character {
        char character;
        char colorAttribute;
    };
    
    VGA_Character* VGA_MEMORY = reinterpret_cast<VGA_Character*>(0xB8000);
    
    int cursorCol = 0;
    int cursorRow = 0;
    
    constexpr unsigned int MAX_COLUMN = 80;
    constexpr unsigned int MAX_ROWS = 25;
    
    constexpr unsigned int VGA_INDEX(const unsigned int col, const unsigned int row) {
        return col + row * MAX_COLUMN;
    }
    
    void clearConsole() {
        for (int i = 0; i < MAX_COLUMN * MAX_ROWS; ++i) {
            VGA_MEMORY[i] = { .character = '\0', .colorAttribute = 0x0 };
        }
    }
    
    void printf(char c) {
        if (cursorRow >= MAX_ROWS) { cursorRow = MAX_ROWS - 1; }
        if (cursorCol >= MAX_COLUMN) { cursorCol = 0; ++cursorRow; }
    
        if (c == '\n') {
            ++cursorRow;
            cursorCol = 0;
            return;
        }
    
        VGA_MEMORY[VGA_INDEX(cursorCol, cursorRow)] = { .character = c, .colorAttribute = 0x0F };
    
        ++cursorCol;
    }

    void printf(const char* str) {
        for (; *str != '\0'; ++str) {
            printf(*str);
        }
    }
   
    void printf(unsigned int n) {
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
    
    void printf(int n) {
        if (n < 0) {
            printf("-");
            n = -n;
        }

        printf(static_cast<unsigned int>(n));
    }

}
