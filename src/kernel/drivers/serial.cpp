#include "axiom/drivers/serial.hpp"
#include "axiom/arch/x64/io.hpp"

namespace axiom::serial {

constexpr uint16_t COM1 = 0x3F8;

void init() {
    using namespace arch::x64;
    outb(COM1 + 1, 0x00);    // Disable all interrupts
    outb(COM1 + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(COM1 + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    outb(COM1 + 1, 0x00);    //                  (hi byte)
    outb(COM1 + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(COM1 + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    // outb(COM1 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

int is_transmit_empty() {
    return axiom::arch::x64::inb(COM1 + 5) & 0x20;
}

void putchar(char c) {
    while (is_transmit_empty() == 0);
    axiom::arch::x64::outb(COM1, c);
}

void puts(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        putchar(str[i]);
    }
}

void puthex(uint64_t val) {
    const char* hex = "0123456789ABCDEF";
    puts("0x");
    for (int i = 60; i >= 0; i -= 4) {
        putchar(hex[(val >> i) & 0xF]);
    }
}

} // namespace axiom::serial
