// Finally Some C++

#include <stdint.h>
#include "kernel/kernel-config.hpp"
#include "kernel/utils/math.hpp"
#include "kernel/utils/memory.hpp"
#include "kernel/memory/internals/pmm.hpp"
#include "kernel/memory/internals/vmm.hpp"
#include "kernel/memory/internals/memory-map.hpp"
#include "kernel/memory/memory-defs.hpp"
#include "kernel/memory/memory-unit.hpp"
#include "kernel/memory/memory-manager.hpp"
#define HANG() while (true)

// ── VGA ──────────────────────────────────────────────────────────────────────

struct VGA_Character {
    char character;
    char colorAttribute;
};

VGA_Character* VGA_MEMORY = reinterpret_cast<VGA_Character*>(0xB8000);

int cursorCol = 0;
int cursorRow = 0;

constexpr unsigned int MAX_COLUMN = 80;
constexpr unsigned int MAX_ROWS = 25;

static void AdvanceCursor() {
    ++cursorCol;
    if (cursorRow >= MAX_ROWS) { cursorRow = MAX_ROWS - 1; }
    if (cursorCol >= MAX_COLUMN) { cursorCol = 0; ++cursorRow; }
}

constexpr unsigned int VGA_INDEX(const unsigned int col, const unsigned int row) {
    return col + row * MAX_COLUMN;
}

void kernel_printf(char c) {
    if (c == '\0') return;
    if (c == '\t') {
        int nextTab = (cursorCol + 4) & ~3;
        while (cursorCol < nextTab) kernel_printf(' ');
        return;
    }
    if (c == '\n') {
        VGA_MEMORY[VGA_INDEX(cursorCol, cursorRow)] = { };
        ++cursorRow;
        if (cursorRow >= MAX_ROWS) cursorRow = MAX_ROWS - 1;
        cursorCol = 0;
        return;
    }
    if (c == '\r') {
        cursorCol = 0;
        return;
    }
    VGA_MEMORY[VGA_INDEX(cursorCol, cursorRow)] = { .character = c, .colorAttribute = 0x0F };
    AdvanceCursor();
}

void kernel_printf(const char* str) {
    for (; *str != '\0'; ++str) kernel_printf(*str);
}

void kernel_printf(uint32_t n) {
    if (n == 0) { kernel_printf('0'); return; }
    char buffer[10];
    int i = 0;
    while (n > 0) { buffer[i++] = (n % 10) + '0'; n = n / 10; }
    while (i > 0) { kernel_printf(buffer[--i]); }
}

void kernel_printf(uint64_t n) {
    if (n == 0) { kernel_printf('0'); return; }
    char buffer[20];
    int i = 0;
    while (n > 0) { buffer[i++] = (n % 10) + '0'; n = n / 10; }
    while (i > 0) { kernel_printf(buffer[--i]); }
}

void kernel_printf(double d) {
    if (d != d) { kernel_printf("NaN"); return; }
    if (d < -1e308) { kernel_printf("-Inf"); return; }
    if (d > 1e308)  { kernel_printf("Inf");  return; }
    if (d > 1e19) { kernel_printf("(too large)"); return; }
    if (d < 0) { kernel_printf('-'); d = -d; }
    kernel_printf(static_cast<uint64_t>(d));
    kernel_printf('.');
    d -= static_cast<uint64_t>(d);
    for (int i = 0; i < 6; ++i) {
        d *= 10;
        kernel_printf(static_cast<char>('0' + static_cast<uint64_t>(d) % 10));
    }
}

void kernel_printf(uint16_t n) { kernel_printf(static_cast<uint32_t>(n)); }
void kernel_printf(int n) {
    if (n < 0) { kernel_printf('-'); n = -n; }
    kernel_printf(static_cast<unsigned int>(n));
}

void kernel_printfHex(uint64_t n) {
    kernel_printf('0'); kernel_printf('x');
    for (int i = 60; i >= 0; i -= 4) {
        uint64_t digit = (n >> i) & 0xF;
        kernel_printf(static_cast<char>(digit < 10 ? '0' + digit : 'A' + digit - 10));
    }
}

void kernel_printfHex(uint32_t n) {
    kernel_printf('0'); kernel_printf('x');
    for (int i = 28; i >= 0; i -= 4) {
        uint32_t digit = (n >> i) & 0xF;
        kernel_printf(static_cast<char>(digit < 10 ? '0' + digit : 'A' + digit - 10));
    }
}

void kernel_clearConsole() {
    for (int i = 0; i < MAX_COLUMN * MAX_ROWS; ++i)
        VGA_MEMORY[i] = { .character = '\0', .colorAttribute = 0x0 };
}

template<typename First, typename... Others>
void kernel_printf(const First& first, const Others&... others) {
    kernel_printf(first);
    kernel_printf(others...);
}

void copyBytes(void* src, void* dst, uint64_t byteNum) {
    uint8_t* s = reinterpret_cast<uint8_t*>(src);
    uint8_t* d = reinterpret_cast<uint8_t*>(dst);
    for (uint64_t i = 0; i < byteNum; ++i) {
        d[i] = s[i];
    }
}

using namespace kernel;

void MapAllMemory_2MB() {
    uint64_t highestAddress = 0;
    for (uint32_t i = 0; i < MEMORY_MAP_ENTRY_COUNT; ++i) {
        const E820Entry entry = E820Entries[i];
        if (entry.type != EntryType::Usable) continue;
        uint64_t end = entry.base + entry.length;
        if (end > highestAddress) highestAddress = end;
    }
    
    uint64_t pdNum = (highestAddress + 0x3FFFFFFF) / 0x40000000;
    
    // Temp tables in already-mapped low memory
    uint64_t* temp_pml4 = reinterpret_cast<uint64_t*>(0x50000);
    uint64_t* temp_pdpt = reinterpret_cast<uint64_t*>(0x51000);
    // PDs start at 0x52000, one 4KB page each
    
    constexpr uint64_t TABLE_ENTRIES_NUM = 512;
    memset(temp_pml4, 0, TABLE_ENTRIES_NUM * 8);
    memset(temp_pdpt, 0, TABLE_ENTRIES_NUM * 8);
    temp_pml4[0] = reinterpret_cast<uint64_t>(temp_pdpt) | 0x3;
    
    for (uint64_t i = 0; i < pdNum; i++) {
        uint64_t* temp_pd = reinterpret_cast<uint64_t*>(0x52000 + i * 0x1000);
        memset(temp_pd, 0, TABLE_ENTRIES_NUM * 8);
        temp_pdpt[i] = reinterpret_cast<uint64_t>(temp_pd) | 0x3;
        for (uint64_t j = 0; j < 512; j++)
            temp_pd[j] = ((i * 0x40000000ULL) + (j * 0x200000ULL)) | 0x83;
    }
    
    asm volatile("mov %0, %%cr3" :: "r"(temp_pml4) : "memory");

}

extern "C" void kernel_entry() {
    kernel_clearConsole();
    kernel_printf("Kernel entry reached\n");

    kernel_printf("Temporarily mapping whole memory");
    MapAllMemory_2MB();

    uint8_t* check = reinterpret_cast<uint8_t*>(TEMP_KERNEL_MAIN_LOAD_ADDR);
    kernel_printf("[VERIFY] First bytes: ");
    kernel_printfHex((uint32_t)check[0]);
    kernel_printf(" ");
    kernel_printfHex((uint32_t)check[1]);
    kernel_printf(" ");
    kernel_printfHex((uint32_t)check[2]);
    kernel_printf("\n");
    
    kernel_printf("Copying kernel to final address: ");
    kernel_printfHex((uint32_t)KERNEL_MAIN_LOAD_ADDR);

    uint64_t kernelSizeBytes = KERNEL_MAIN_SECTORS * 512;
    copyBytes(reinterpret_cast<void*>(TEMP_KERNEL_MAIN_LOAD_ADDR), reinterpret_cast<void*>(KERNEL_MAIN_LOAD_ADDR), kernelSizeBytes);

    kernel_printf("\nAfter copy to: ", "0x00100000", "\n[VERIFY] First bytes: ");
    check = reinterpret_cast<uint8_t*>(KERNEL_MAIN_LOAD_ADDR);
    kernel_printf("[VERIFY] First bytes: ");
    kernel_printfHex((uint32_t)check[0]);
    kernel_printf(" ");
    kernel_printfHex((uint32_t)check[1]);
    kernel_printf(" ");
    kernel_printfHex((uint32_t)check[2]);
    kernel_printf("\n");

    MemoryManager memoryManager{};
    memoryManager.Init();

    kernel_printf("[VERIFY] First bytes after cr3 switch: ");
    check = reinterpret_cast<uint8_t*>(KERNEL_VIRT_BASE);
    kernel_printf("[VERIFY] First bytes: ");
    kernel_printfHex((uint32_t)check[0]);
    kernel_printf(" ");
    kernel_printfHex((uint32_t)check[1]);
    kernel_printf(" ");
    kernel_printfHex((uint32_t)check[2]);
    kernel_printf("\n");

    typedef void (*KernelMain)();
    KernelMain kernelMain = reinterpret_cast<KernelMain>(KERNEL_VIRT_BASE);
    kernelMain();

    // Should never reach here
    kernel_printf("Returned from kernelMain — halting\n");
    HANG();
}
