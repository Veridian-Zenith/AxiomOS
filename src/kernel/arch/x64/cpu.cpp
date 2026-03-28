#include "axiom/arch/x64/cpu.hpp"
#include "axiom/arch/x64/private/gdt.hpp"

namespace axiom::arch::x64 {

void setupGdt() {
    initGdt();
}

void setupIdt() {
    // TODO: Implement IDT setup
}

void halt() {
    __asm__ volatile("hlt");
}

} // namespace axiom::arch::x64
