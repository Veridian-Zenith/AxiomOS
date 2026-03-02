# AxiomOS Development Prompt

## Project Overview

Build a custom operating system named AxiomOS using Zig 0.15.2 as the primary implementation language. The OS targets x86_64 (UEFI/Secure Boot capable), starting with the HP ProBook 450 G9 (12th Gen Intel i3-1215U, Intel Xe graphics, NVMe storage). Borrow Linux drivers/firmware where practical. Future GUI will use Flutter or similar. Custom package format: .zv (ZenithVoid).
Target Hardware Specifications
plain
Copy
Platform: x86_64 UEFI (Secure Boot capable)
CPU: Intel 12th Gen Core i3-1215U (6-core: 2P+4E, Alder Lake)
GPU: Intel UHD Graphics (Xe driver, Vulkan 1.4, OpenGL 4.6)
RAM: 16GB DDR4
Storage: 238GB NVMe (KIOXIA)
Network: Realtek RTL8852BE (WiFi 6), RTL8111 (Ethernet)
Audio: Intel Alder Lake PCH + SOF firmware
Boot: UEFI with Secure Boot support
Cryptography Standards
Primary Hash: SHA-384
Fallback Hash: SHA-256 (for legacy/compatibility)
Primary Encryption: ED25519 (256-bit)
Build Environment (from config.fish)
fish
Copy

## Compiler Toolchain

CC=clang, CXX=clang++, LD=ld.lld, AR=llvm-ar
CFLAGS="-march=native -O3 -pipe -fno-plt"
LDFLAGS="-fuse-ld=mold -Wl,-O1,--as-needed,-z,relro,-z,now"
CPPFLAGS="-D_FORTIFY_SOURCE=3"
LTOFLAGS="-flto=thin"

## Rust (for future components)

RUSTFLAGS="-C linker=clang -C link-arg=-fuse-ld=lld -C lto=thin"
CARGO_INCREMENTAL=0

## CMake/Make

CMAKE_C_FLAGS="$CFLAGS", MAKEFLAGS="-s"
Project Structure
plain
Copy
AxiomOS/
├── build.zig              # Main build configuration
├── build.zig.zon          # Dependency management
├── docs/                  # Architecture & design docs
├── kernel/                # Core kernel (Zig)
│   ├── src/
│   │   ├── main.zig       # Entry point
│   │   ├── arch/          # x86_64 specific
│   │   ├── mm/            # Memory management
│   │   ├── sched/         # Scheduler
│   │   └── drivers/       # Borrowed Linux drivers (C)
│   └── build.zig
├── bootloader/            # UEFI bootloader (Zig)
├── libaxc/                # Axiom C library (libc replacement)
├── pkg/                   # Package manager (.zv format)
├── userspace/             # Userland utilities
└── tools/                 # Build tools & generators
Implementation Requirements
Phase 1: Foundation (Current)
UEFI Bootloader
Parse EFI system tables
Load kernel to proper memory region
Set up basic page tables (4-level paging)
Exit boot services and handoff to kernel
Kernel Core (64-bit long mode)
GDT/IDT setup
Basic UART serial output (for debugging)
Physical memory manager (bitmap or buddy allocator)
Virtual memory manager (page tables)
Interrupt handling (PIC/APIC setup)
Basic timer (PIT/HPET/TSC)
Build System
Cross-compilation setup in build.zig
Kernel linker script (higher half kernel)
ISO/USB image generation
Debug symbols preservation
Phase 2: Essential Drivers (Borrow from Linux)
Storage: NVMe driver (port from Linux or use ZBSS)
Graphics: Basic framebuffer (Intel Xe later)
Input: PS/2 keyboard (USB HID later)
Network: RTL8852BE firmware loading
Phase 3: Userspace
System call interface
libaxc (POSIX subset + Axiom extensions)
Basic shell
.zv package format parser
Code Quality Standards
zig
Copy
// Every file must start with:
//! Module description: what this file does
//! Architecture: x86_64
//! Safety: requirements/assumptions

// Every function must have:
/// Brief description
/// Parameters: name - description
/// Returns: description
/// Safety: caller requirements (unsafe blocks)
/// Example: (if applicable)
fn functionName(param: Type) ReturnType {
    // Implementation with inline comments for non-obvious logic
}
Zig 0.15.2 Specific Requirements
Use std.Build API for build scripts
Target x86_64-freestanding-none or x86_64-uefi
Use @import("std").os.uefi for bootloader
Kernel: pure freestanding, no OS abstraction
Explicit error handling with try/catch
Avoid usingnamespace in kernel code (namespace pollution)
Security & Hardening
Stack canaries (-fstack-protector-strong equivalent)
Position Independent Code (PIE) where possible
RELRO (Relocation Read-Only)
NX bit enforcement
Safe integer arithmetic (checked overflow)
Constant-time crypto primitives (for future use)
Deliverables for This Session
Complete build.zig with proper cross-compilation
UEFI bootloader skeleton with GOP framebuffer init
Kernel entry point with long mode transition
Basic printk-style logging via serial
Physical memory map parsing from UEFI
Simple page allocator (bump allocator acceptable for now)
Documentation References
Primary: <https://ziglang.org/documentation/0.15.2/>
UEFI Spec: 2.10 or later
Intel SDM: Volume 3 (System Programming)
Linux: drivers/nvme/, drivers/net/wireless/realtek/rtw89/
Notes
Start minimal. A working "Hello Kernel" that boots via UEFI and prints to serial is a valid first milestone.
Document EVERY hardware assumption.
When borrowing Linux drivers, create Zig shims for the C interfaces.
Plan for SMP (multi-core) from day one (ACPI MADT parsing), even if single-core initially.
