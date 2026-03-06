#ifndef AXIOMOS_X86_64_HPP
#define AXIOMOS_X86_64_HPP

#include <stdint.h>

namespace axiom::arch::x86_64 {

// ====================================================================
// CPU Instructions & Control Registers
// ====================================================================

/// @brief Halt the CPU until the next interrupt
inline void halt() {
    __asm__ volatile("hlt");
}

/// @brief Enable hardware interrupts
inline void enableInterrupts() {
    __asm__ volatile("sti");
}

/// @brief Disable hardware interrupts
inline void disableInterrupts() {
    __asm__ volatile("cli");
}

/// @brief Read the Time Stamp Counter (TSC)
inline uint64_t readTsc() {
    uint32_t low, high;
    __asm__ volatile("rdtsc" : "=a"(low), "=d"(high));
    return (static_cast<uint64_t>(high) << 32) | low;
}

/// @brief Read CR0 (Control Register 0 - PE, PG, etc.)
inline uint64_t readCr0() {
    uint64_t val;
    __asm__ volatile("mov %%cr0, %0" : "=r"(val));
    return val;
}

/// @brief Write CR0
inline void writeCr0(uint64_t val) {
    __asm__ volatile("mov %0, %%cr0" : : "r"(val) : "memory");
}

/// @brief Read CR2 (Page Fault Linear Address)
inline uint64_t readCr2() {
    uint64_t val;
    __asm__ volatile("mov %%cr2, %0" : "=r"(val));
    return val;
}

/// @brief Read CR3 (Page Directory Base Register)
inline uint64_t readCr3() {
    uint64_t val;
    __asm__ volatile("mov %%cr3, %0" : "=r"(val));
    return val;
}

/// @brief Write CR3 (Switches page tables/flushes TLB)
inline void writeCr3(uint64_t val) {
    __asm__ volatile("mov %0, %%cr3" : : "r"(val) : "memory");
}

// ====================================================================
// Global Descriptor Table (GDT)
// ====================================================================

struct GdtEntry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;
    uint8_t  limit_high_and_flags;
    uint8_t  base_high;
} __attribute__((packed));

struct GdtDescriptor {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

constexpr uint16_t GDT_KERNEL_CODE = 0x08;
constexpr uint16_t GDT_KERNEL_DATA = 0x10;
constexpr uint16_t GDT_USER_CODE   = 0x18;
constexpr uint16_t GDT_USER_DATA   = 0x20;

/// @brief Initialize and load the flat 64-bit GDT
void setupGdt();

// ====================================================================
// Interrupt Descriptor Table (IDT)
// ====================================================================

struct IdtEntry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  ist;        // Interrupt Stack Table offset
    uint8_t  type_attr;  // Type and Attributes
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t reserved;
} __attribute__((packed));

struct IdtDescriptor {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

/// @brief Initialize and load a zeroed IDT
void setupIdt();

// ====================================================================
// Legacy PIC Initialization
// ====================================================================

/// @brief Remap the legacy 8259 PIC vectors to avoid CPU exceptions
/// @note Even though the i3-1215U has an APIC, the legacy PIC must be 
/// properly remapped and masked to prevent spurious interrupts during boot.
void initPic();

} // namespace axiom::arch::x86_64

#endif // AXIOMOS_X86_64_HPP