#ifndef AXIOMOS_UTILS_SERIAL_HPP
#define AXIOMOS_UTILS_SERIAL_HPP

#include <stdint.h>
#include <stddef.h>

namespace axiom::serial {

// COM1 Base Port
constexpr uint16_t COM1 = 0x3F8;

// Initialize the UART for serial output (115200 baud, 8N1)
void init();

// Write a single character to the serial port
void putchar(char c);

// Write a null-terminated string
void puts(const char* str);

// Minimal printf implementation
void printf(const char* format, ...);

} // namespace axiom::serial

#endif // AXIOMOS_UTILS_SERIAL_HPP

