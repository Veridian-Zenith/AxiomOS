#include "axiom/arch/x64/cpu.hpp"
#include "axiom/arch/x64/idt.hpp"
#include "axiom/arch/x64/private/gdt.hpp"

namespace axiom::arch::x64 {

void setupGdt() {
    initGdt();
}

void setupIdt() {
    initIdt();
}

void halt() {
    __asm__ volatile("hlt");
}

} // namespace axiom::arch::x64
