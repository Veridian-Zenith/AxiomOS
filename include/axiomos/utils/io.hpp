// Module description: Low-level I/O port operations for x86_64.
// Architecture: x86_64
// Safety: Direct hardware access. Unsafe.
#ifndef AXIOMOS_UTILS_IO_HPP
#define AXIOMOS_UTILS_IO_HPP

#include <stdint.h>

namespace axiom::utils::io {

/// @brief Output a byte to an I/O port
/// @param port The I/O port address
/// @param value The byte to write
/// @note Safety: Requires ring 0 (kernel mode) and valid I/O port mapping
inline void outb(uint16_t port, uint8_t value) {
    // Target: Clang/LLVM x86_64
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

/// @brief Input a byte from an I/O port
/// @param port The I/O port address
/// @return The byte read from the port
/// @note Safety: Requires ring 0 (kernel mode) and valid I/O port mapping
inline uint8_t inb(uint16_t port) {
    uint8_t value;
    // Target: Clang/LLVM x86_64
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

/// @brief Delay execution briefly using a dummy I/O write
inline void ioWait() {
    outb(0x80, 0);
}

} // namespace axiom::utils::io

#endif // AXIOMOS_UTILS_IO_HPP
