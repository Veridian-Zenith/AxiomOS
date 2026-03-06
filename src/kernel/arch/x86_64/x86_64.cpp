// Module description: x86_64 architecture-specific setup (GDT, IDT, PIC).
// Architecture: x86_64
// Safety: Functions in this module are unsafe and must be called
//         only during kernel initialization in the correct order.
#include "axiomos/x86_64.hpp"
#include "axiomos/utils/io.hpp"

namespace axiom::arch::x86_64 {

// ====================================================================
// Global Descriptor Table (GDT)
// ====================================================================

namespace {
    // 5 entries: Null, KCode, KData, UCode, UData
    alignas(16) GdtEntry gdt[5];
    GdtDescriptor gdt_descriptor;
}

void setupGdt() {
    // 0: Null Descriptor
    gdt[0] = {0, 0, 0, 0, 0, 0};

    // 1: Kernel Code (64-bit)
    gdt[1] = {
        .limit_low = 0xFFFF,
        .base_low = 0,
        .base_mid = 0,
        .access = 0x9A, // Present (1), DPL (00), System (1), Code (1), Conforming (0), Readable (1), Accessed (0)
        .limit_high_and_flags = 0x20, // 64-bit flag (1), limit_high (0)
        .base_high = 0
    };

    // 2: Kernel Data (64-bit)
    gdt[2] = {
        .limit_low = 0xFFFF,
        .base_low = 0,
        .base_mid = 0,
        .access = 0x92, // Present (1), DPL (00), System (1), Data (0), Direction (0), Writable (1), Accessed (0)
        .limit_high_and_flags = 0x00,
        .base_high = 0
    };

    // 3: User Code (64-bit)
    gdt[3] = {
        .limit_low = 0xFFFF,
        .base_low = 0,
        .base_mid = 0,
        .access = 0xFA, // DPL=3 (11)
        .limit_high_and_flags = 0x20,
        .base_high = 0
    };

    // 4: User Data (64-bit)
    gdt[4] = {
        .limit_low = 0xFFFF,
        .base_low = 0,
        .base_mid = 0,
        .access = 0xF2, // DPL=3 (11)
        .limit_high_and_flags = 0x00,
        .base_high = 0
    };

    gdt_descriptor.limit = sizeof(gdt) - 1;
    gdt_descriptor.base = reinterpret_cast<uint64_t>(&gdt[0]);

    // Load GDT
    __asm__ volatile("lgdt %0" : : "m"(gdt_descriptor));

    // Reload Data Segments
    __asm__ volatile(
        "movw %0, %%ax \n"
        "movw %%ax, %%ds \n"
        "movw %%ax, %%es \n"
        "movw %%ax, %%fs \n"
        "movw %%ax, %%gs \n"
        "movw %%ax, %%ss \n"
        : : "i"(GDT_KERNEL_DATA) : "ax"
    );

    // Reload Code Segment via far return
    // (pushing segment selector and return address, then retfq)
    __asm__ volatile(
        "pushq %0 \n"
        "leaq 1f(%%rip), %%rax \n"
        "pushq %%rax \n"
        "lretq \n"
        "1: \n"
        : : "i"(GDT_KERNEL_CODE) : "rax", "memory"
    );
}

// ====================================================================
// Interrupt Descriptor Table (IDT)
// ====================================================================

namespace {
    alignas(4096) IdtEntry idt[256];
    IdtDescriptor idt_descriptor;
}

void setupIdt() {
    for (int i = 0; i < 256; ++i) {
        idt[i] = {0, 0, 0, 0, 0, 0, 0};
    }

    idt_descriptor.limit = sizeof(idt) - 1;
    idt_descriptor.base = reinterpret_cast<uint64_t>(&idt[0]);

    __asm__ volatile("lidt %0" : : "m"(idt_descriptor));
}

// ====================================================================
// Legacy Programmable Interrupt Controller (PIC)
// ====================================================================

namespace {
    constexpr uint16_t PIC1_CMD  = 0x20;
    constexpr uint16_t PIC1_DATA = 0x21;
    constexpr uint16_t PIC2_CMD  = 0xA0;
    constexpr uint16_t PIC2_DATA = 0xA1;

    constexpr uint8_t ICW1_INIT = 0x10;
    constexpr uint8_t ICW1_ICW4 = 0x01;
    constexpr uint8_t ICW4_8086 = 0x01;
}

void initPic() {
    using namespace axiom::utils::io;

    // Restart PICs (ICW1)
    outb(PIC1_CMD, ICW1_INIT | ICW1_ICW4);
    ioWait();
    outb(PIC2_CMD, ICW1_INIT | ICW1_ICW4);
    ioWait();

    // Map Master PIC IRQs to 0x20-0x27 (ICW2)
    outb(PIC1_DATA, 0x20);
    ioWait();

    // Map Slave PIC IRQs to 0x28-0x2F (ICW2)
    outb(PIC2_DATA, 0x28);
    ioWait();

    // Setup cascading (ICW3)
    outb(PIC1_DATA, 0x04); // Tell Master there is a slave at IRQ2
    ioWait();
    outb(PIC2_DATA, 0x02); // Tell Slave its cascade identity
    ioWait();

    // 8086 mode (ICW4)
    outb(PIC1_DATA, ICW4_8086);
    ioWait();
    outb(PIC2_DATA, ICW4_8086);
    ioWait();

    // Mask all interrupts for now.
    // We will unmask them when we set up the APIC/IOAPIC later,
    // since modern Intel hardware (like the i3-1215U) relies heavily on APIC.
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
}

} // namespace axiom::arch::x86_64
