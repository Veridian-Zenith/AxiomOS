#pragma once
#include <stdint.h>

namespace axiom::serial {
    void init();
    void puts(const char* str);
    void putchar(char c);
    void puthex(uint64_t val);
}