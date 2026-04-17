#pragma once
#include <cstdint>

namespace axiom::syscall {

// Syscall codes
enum class Code : uint64_t {
    Exit = 0,
    Yield,
    Write,
    Read,
    IpcSend,
    IpcReceive
};

// Syscall invocation
// Using inline assembly for syscalls is standard in OS dev
inline uint64_t Invoke(Code code, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
    uint64_t result;
    asm volatile (
        "syscall"
        : "=a"(result)
        : "a"(static_cast<uint64_t>(code)), "D"(arg1), "S"(arg2), "d"(arg3)
        : "rcx", "r11", "memory"
    );
    return result;
}

} // namespace axiom::syscall
