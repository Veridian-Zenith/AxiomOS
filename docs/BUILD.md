# Building AxiomOS

## Prerequisites

- Zig 0.15.2+
- LLVM/Clang toolchain (from config.fish)
- mold linker (optional, but recommended)
- QEMU x86_64 (for testing)

## Setup Environment

```bash
# Load build environment
source config.fish

# Verify Zig installation
zig version

# Check available targets
zig targets | grep x86_64
```

## Build Commands

### Full Build
```bash
zig build
```

### Build Bootloader Only
```bash
zig build-exe bootloader/main.zig -target x86_64-uefi -O ReleaseSafe
```

### Build Kernel Only
```bash
zig build-exe kernel/src/main.zig -target x86_64-freestanding -O ReleaseSafe
```

### Run in QEMU

```bash
# Boot with bootloader (requires disk image setup)
qemu-system-x86_64 \
  -bios /usr/share/ovmf/OVMF_CODE.fd \
  -cpu host \
  -m 4G \
  -serial stdio \
  -drive format=raw,file=axiom.img
```

## Code Quality Checks

### Lint with zig fmt
```bash
zig fmt --check kernel/src/
zig fmt kernel/src/  # Auto-fix
```

## Debug Build Artifacts

```bash
# Inspect kernel symbols
llvm-nm zig-out/bin/kernel

# Disassemble kernel
llvm-objdump -d zig-out/bin/kernel | head -100

# Check sections
llvm-objdump -h zig-out/bin/kernel
```

## Troubleshooting

**Error: "cannot find -lc"**
- Expected, we're using freestanding target

**Serial output not showing**
- Check COM1 (0x3F8) is accessible
- Verify UART initialization in serial.zig
- Use QEMU's -serial option

**Kernel panics at startup**
- Check memory map parsing (check EFI descriptor count)
- Verify higher-half kernel mapping (linker.ld)
