#include <stdint.h>

// functions to display text
volatile uint16_t* vga_buffer = (uint16_t*)0xB8000;
const int VGA_WIDTH = 80;
const int VGA_HEIGHT = 25;

void write_char(int x, int y, char c, uint8_t color) {
    vga_buffer[y * VGA_WIDTH + x] = (color << 8) | c;
}

extern "C" void kernel_main() {
    // Clear screen to black background, white text
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            write_char(x, y, ' ', 0x0F);
        }
    }

    // Write "Hello, world!" starting at (0,0) in white text
    const char* message = "Hello, from kernel!";
    int i = 0;
    while (message[i] != '\0') {
        write_char(i, 0, message[i], 0x0F);
        i++;
    }

    // Keep the CPU busy (infinite loop)
    while (1) {}
}
