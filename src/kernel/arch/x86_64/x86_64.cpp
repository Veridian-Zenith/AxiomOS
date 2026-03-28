#include "axiomos/x86_64.hpp"

namespace axiom::arch::x86_64 {

void setupGdt() {
    // TODO: Implement GDT setup
}

void setupIdt() {
    // TODO: Implement IDT setup
}

void halt() {
    __asm__ volatile("hlt");
}

} // namespace axiom::arch::x86_64
