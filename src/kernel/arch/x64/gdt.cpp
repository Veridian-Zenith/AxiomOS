#include "axiom/arch/x64/private/gdt.hpp"

namespace axiom::arch::x64 {

struct Gdt {
    GdtEntry entries[5];
    GdtTssEntry tss;
} __attribute__((packed, aligned(8)));

static Gdt gdt;
static TssEntry tss;

extern "C" void gdt_load(uint64_t ptr_addr, uint16_t code, uint16_t data);
extern "C" void tss_load(uint16_t sel);

void initGdt() {
    // 1. Setup GDT Entries
    // Null Descriptor
    gdt.entries[0] = {0, 0, 0, 0, 0, 0};

    // Kernel Code 64 (0x08)
    gdt.entries[1] = {0, 0, 0, 0x9A, 0x20, 0};

    // Kernel Data 64 (0x10)
    gdt.entries[2] = {0, 0, 0, 0x92, 0x00, 0};

    // User Data 64 (0x18 -> 0x1B)
    gdt.entries[3] = {0, 0, 0, 0xF2, 0x00, 0};

    // User Code 64 (0x20 -> 0x23)
    gdt.entries[4] = {0, 0, 0, 0xFA, 0x20, 0};

    // 2. Setup TSS Descriptor (0x28)
    uint64_t tss_base = (uint64_t)&tss;
    gdt.tss.limit_low = sizeof(TssEntry);
    gdt.tss.base_low = (uint16_t)(tss_base & 0xFFFF);
    gdt.tss.base_middle = (uint8_t)((tss_base >> 16) & 0xFF);
    gdt.tss.access = 0x89; // Present, Executable, 64-bit TSS (Available)
    gdt.tss.granularity = 0x00;
    gdt.tss.base_high = (uint8_t)((tss_base >> 24) & 0xFF);
    gdt.tss.base_upper = (uint32_t)((tss_base >> 32) & 0xFFFFFFFF);
    gdt.tss.reserved = 0;

    // 3. Initialize TSS with default values
    for (int i = 0; i < (int)sizeof(TssEntry); i++) ((uint8_t*)&tss)[i] = 0;
    tss.iopb_offset = sizeof(TssEntry);

    // 4. Load the GDT
    GdtPointer ptr;
    ptr.limit = sizeof(Gdt) - 1;
    ptr.base = (uint64_t)&gdt;

    gdt_load((uint64_t)&ptr, KERNEL_CODE_SEL, KERNEL_DATA_SEL);
    tss_load(TSS_SEL);
}

} // namespace axiom::arch::x64
