#pragma once
#include <stdint.h>

namespace axiom::arch::x64 {
    void setupGdt();
    void setupIdt();
    void setupApic();
    void halt();
}