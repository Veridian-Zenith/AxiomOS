# AxiomOS Architecture & Implementation Notes

## Project Structure

```
AxiomOS/
├── bootloader/           # UEFI bootloader
│   ├── main.zig          # Entry point, GOP init, memory map
│   └── ...
├── kernel/               # x86_64 kernel core
│   ├── src/
│   │   ├── main.zig      # Kernel entry (kinit)
│   │   ├── serial.zig    # UART output
│   │   ├── arch/         # Architecture-specific
│   │   │   ├── x86_64.zig    # CPU instructions, GDT, IDT, PIC
│   │   │   └── entry.zig     # Assembly entry stub
│   │   ├── mm/           # Memory management
│   │   │   ├── allocator.zig # Physical page allocator
│   │   │   └── paging.zig    # Virtual memory (TODO)
│   │   ├── sched/        # Scheduler (TODO)
│   │   └── drivers/      # Device drivers (TODO)
│   ├── linker.ld         # Higher-half kernel linker script
│   └── build.zig         # Kernel build config
├── build.zig             # Root build system
├── build.zig.zon         # Project manifest
└── docs/                 # Documentation
```

## Boot Flow

1. **UEFI Firmware** loads bootloader.efi from ESP
2. **Bootloader (Zig)**:
   - Initializes GOP framebuffer
   - Collects EFI memory map
   - Loads kernel to ENTRY_POINT (0xffffffff80000000)
   - Exits boot services
   - Jumps to kernel entry
3. **Kernel (Zig)**:
   - Initializes serial output (debug)
   - Sets up GDT/IDT
   - Initializes interrupt controller (PIC/APIC)
   - Parses physical memory
   - Initializes page allocator
   - Ready for userspace

## Deliverables (Phase 1)

- [x] build.zig with x86_64 cross-compilation
- [x] UEFI bootloader with GOP framebuffer
- [x] Kernel entry point with long mode setup
- [x] Serial logging (UART 8250/16550)
- [x] EFI memory map parsing
- [x] Physical memory allocator (bitmap-based)

## Build Instructions

```bash
# Build bootloader & kernel
zig build

# Clean build
zig build clean

# Build specific target
zig build --step-name bootloader
```

## Debugging

Serial console (COM1, 115200 baud):
```bash
picocom -b 115200 /dev/ttyUSB0
# or
screen /dev/ttyUSB0 115200
```

## Next Steps (Phase 2)

- Implement virtual memory (4-level paging)
- NVMe driver (port from Linux or ZBSS)
- Basic framebuffer graphics
- PS/2 keyboard input
- Network RTL driver firmware loading

## Hardware Assumptions

- x86_64 CPU with 64-bit long mode
- UEFI firmware (BIOS not supported)
- 4K page size
- Intel Xe GPU (future)
- KIOXIA NVMe storage
