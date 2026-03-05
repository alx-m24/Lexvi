// Finally Some C++

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

void kernel_printf(const char* str) {
    while (*str != '\0') {
        if (cursorRow >= MAX_ROWS) { cursorRow = MAX_ROWS - 1; }
        if (cursorCol >= MAX_COLUMN) { cursorCol = 0; ++cursorRow; }

        if (*str == '\n') {
            ++cursorRow;
            cursorCol = 0;
            ++str;
            continue;
        }

        VGA_MEMORY[VGA_INDEX(cursorCol, cursorRow)] = { .character = *str, .colorAttribute = 0x0F };

        ++cursorCol; ++str;
    }
}

void kernel_clearConsole() {
    for (int i = 0; i < MAX_COLUMN * MAX_ROWS; ++i) {
        VGA_MEMORY[i] = { .character = '\0', .colorAttribute = 0x0 };
    }
}

inline unsigned char inb(unsigned short port) {
    unsigned char result;
    asm volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

inline void outb(unsigned short port, unsigned char value) {
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

void kernel_Delay400ns() {
    inb(0x1F7);
    inb(0x1F7);
    inb(0x1F7);
    inb(0x1F7);
}

bool kernel_isDriveBusy(unsigned char status) {
    return (status & 0b10000000;
}

bool kernel_isDriveReadyForTransfer(unsigned char status) {
    return status & 0b00001000;
}

void kernel_loadKernelMain() {
    outb(0x1F6, 0xE0);  // select master drive, LBA mode
    outb(0x3F6, 0x04);   // assert SRST bit in device control register
    outb(0x3F6, 0x00);   // clear SRST
    kernel_Delay400ns();

    
    kernel_printf("Waiting for drive...\n");
    while (kernel_isDriveBusy(inb(0x1F7)));
    kernel_printf("Drive ready\n");

    if (kernel_isDriveReadyForTransfer(inb(0x1F7))) kernel_printf("Ready to transfer\n");
}

extern "C" void kernel_entry() {
    kernel_clearConsole(); 
    kernel_printf("Successfully loaded Kernel\n");

    kernel_loadKernelMain();

    while (true);
}
