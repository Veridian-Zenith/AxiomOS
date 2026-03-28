#pragma once
#include <stdint.h>

namespace axiom::arch::x64 {

struct GdtEntry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

struct GdtPointer {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

struct TssEntry {
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iopb_offset;
} __attribute__((packed));

struct GdtTssEntry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
    uint32_t base_upper;
    uint32_t reserved;
} __attribute__((packed));

void initGdt();

// Selectors
constexpr uint16_t KERNEL_CODE_SEL = 0x08;
constexpr uint16_t KERNEL_DATA_SEL = 0x10;
constexpr uint16_t USER_DATA_SEL   = 0x1B; // RPL 3
constexpr uint16_t USER_CODE_SEL   = 0x23; // RPL 3
constexpr uint16_t TSS_SEL         = 0x28;

} // namespace axiom::arch::x64
