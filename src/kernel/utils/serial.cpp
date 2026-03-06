// Module description: Simple serial port driver for COM1 (8250/16550 UART).
// Architecture: x86_64 (due to I/O port usage)
// Safety: This module directly interacts with hardware I/O ports and is
//         inherently unsafe. It should only be used for debugging.
#include "axiomos/utils/serial.hpp"
#include "axiomos/utils/io.hpp"

#include <cstdarg>
#include <cstdint>

namespace axiom::serial {

namespace {

using namespace axiom::utils::io;

// Wait until the transmit buffer is empty
inline void waitForTransmit() {
    while ((inb(COM1 + 5) & 0x20) == 0) {
        // Spin
    }
}

} // anonymous namespace

void init() {
    // Disable interrupts
    outb(COM1 + 1, 0x00);
    // Set DLAB to configure baud rate
    outb(COM1 + 3, 0x80);
    // Set divisor for 115200 baud
    outb(COM1 + 0, 0x01);
    outb(COM1 + 1, 0x00);
    // Clear DLAB, set 8N1 (8 bits, no parity, 1 stop bit)
    outb(COM1 + 3, 0x03);
    // Enable FIFO, clear them, with 14-byte threshold
    outb(COM1 + 2, 0xC7);
    // IRQs enabled, RTS/DSR set
    outb(COM1 + 4, 0x0B);
}

void putchar(char c) {
    waitForTransmit();
    outb(COM1, static_cast<uint8_t>(c));

    // Convert LF to CRLF
    if (c == '\n') {
        waitForTransmit();
        outb(COM1, '\r');
    }
}

void puts(const char* str) {
    for (size_t i = 0; str[i] != '\0'; ++i) {
        putchar(str[i]);
    }
}

// Very simple printf implementation (handles %s, %d, %x for now)
void printf(const char* format, ...) {
    va_list args;
    va_start(args, format);

    for (const char* p = format; *p != '\0'; ++p) {
        if (*p != '%') {
            putchar(*p);
            continue;
        }

        ++p; // Skip '%'
        switch (*p) {
            case 's': {
                const char* s = va_arg(args, const char*);
                puts(s ? s : "(null)");
                break;
            }
            case 'd': {
                int i = va_arg(args, int);
                if (i < 0) {
                    putchar('-');
                    i = -i;
                }
                char buf[16];
                int idx = 0;
                do {
                    buf[idx++] = '0' + (i % 10);
                    i /= 10;
                } while (i > 0);
                while (idx > 0) {
                    putchar(buf[--idx]);
                }
                break;
            }
            case 'x': {
                unsigned int x = va_arg(args, unsigned int);
                char buf[16];
                int idx = 0;
                do {
                    int rem = x % 16;
                    buf[idx++] = (rem < 10) ? ('0' + rem) : ('a' + rem - 10);
                    x /= 16;
                } while (x > 0);
                while (idx > 0) {
                    putchar(buf[--idx]);
                }
                break;
            }
            case '%': {
                putchar('%');
                break;
            }
            default: {
                putchar('%');
                putchar(*p);
                break;
            }
        }
    }

    va_end(args);
}

} // namespace axiom::serial
