#pragma once
#include <stdint.h>

namespace axiom::arch::x64 {

struct IdtEntry {
    uint16_t isr_low;
    uint16_t selector;
    uint8_t  ist;
    uint8_t  attributes;
    uint16_t isr_mid;
    uint32_t isr_high;
    uint32_t reserved;
} __attribute__((packed));

struct IdtPointer {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

struct InterruptFrame {
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} __attribute__((packed));

void initIdt();
void setIdtGate(uint8_t vector, void (*handler)(), uint8_t ist = 0, uint8_t type = 0x8E);

} // namespace axiom::arch::x64
