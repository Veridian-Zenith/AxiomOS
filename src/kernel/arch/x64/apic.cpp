#include "axiom/arch/x64/cpu.hpp"
#include "axiom/arch/x64/io.hpp"

namespace axiom::arch::x64 {

// Default LAPIC base physical address
constexpr uint64_t LAPIC_BASE = 0xFEE00000;

// LAPIC Registers
constexpr uint32_t LAPIC_SVR = 0x0F0; // Spurious Interrupt Vector Register
constexpr uint32_t LAPIC_EOI = 0x0B0; // End of Interrupt Register

void writeLapic(uint32_t reg, uint32_t value) {
    volatile uint32_t* lapic = (volatile uint32_t*)(LAPIC_BASE + reg);
    *lapic = value;
}

uint32_t readLapic(uint32_t reg) {
    volatile uint32_t* lapic = (volatile uint32_t*)(LAPIC_BASE + reg);
    return *lapic;
}

void setupApic() {
    // 1. Enable LAPIC by setting the Spurious Interrupt Vector bit 8
    // We use vector 0xFF for spurious interrupts for now.
    uint32_t svr = readLapic(LAPIC_SVR);
    svr |= 0x100; // Enable bit
    svr |= 0xFF;  // Vector 255
    writeLapic(LAPIC_SVR, svr);
}

void signalEoi() {
    writeLapic(LAPIC_EOI, 0);
}

} // namespace axiom::arch::x64
