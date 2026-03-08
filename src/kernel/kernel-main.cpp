// Finally outside bootloader

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

class Parent {
    public:
        virtual void OutputIdentity() {
            kernel_printf("This is the parent\n");
        }
};

class Derived : public Parent {
    void OutputIdentity() override {
        kernel_printf("This is the child\n");
    }
};

void PrintText(Parent* anyDerived) {
    anyDerived->OutputIdentity(); 
}

extern "C" void kernel_main() {
    kernel_clearConsole();
    kernel_printf("Welcome to the main kernel\n");

    Parent parent {};
    Derived derived {};

    PrintText(&parent);
    PrintText(&derived);

    while (true); 
}
