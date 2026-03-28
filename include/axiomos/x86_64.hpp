#pragma once
#include <stdint.h>

namespace axiom::arch::x86_64 {
    void setupGdt();
    void setupIdt();
    void setupApic();
    void halt();
}