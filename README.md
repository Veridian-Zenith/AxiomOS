# AxiomOS

A modern, **UEFI-only**, 64-bit operating system for x86_64, built from scratch in **C++26** using **LLVM/Clang only** (no GCC).

## Project Goals

- **Modern C++:** Leverage the latest C++26 features for safer, cleaner, and more expressive kernel and userspace code.
- **LLVM-Native:** Built exclusively with the Clang/LLVM toolchain for advanced diagnostics, better code generation, and modern language support.
- **Clean Architecture:** High code quality, thorough documentation, and maintainable design. (At least my best attempts at such.)
- **Custom Tooling:** Long-term goal includes a custom package manager and other native utilities.

## Current Status

The project is in its **early stages** but already boots successfully:

- UEFI bootloader that loads the kernel
- Higher-half kernel (mapped at `0xFFFFFFFF80000000`)
- Basic hardware initialization (GDT, IDT, PIC/APIC)
- Bitmap-based physical memory manager
- Serial logging and debug output
- CPU feature detection

The system currently boots to a serial console, prints system information, and halts.

## Project Structure

```fish
.
├── CMakeLists.txt
├── config.fish
├── docs
│   ├── hardware
│   │   ├── cpu.txt
│   │   ├── disks.txt
│   │   ├── memory.txt
│   │   ├── pci.txt
│   │   └── usb.txt
│   └── ROADMAP.md
├── firmware
│   ├── OVMF_CODE.fd
│   └── OVMF_VARS.fd
├── include
│   ├── axiom
│   │   ├── arch
│   │   │   └── x64
│   │   │       ├── cpu.hpp
│   │   │       ├── idt.hpp
│   │   │       ├── io.hpp
│   │   │       └── private
│   │   │           └── gdt.hpp
│   │   ├── boot
│   │   │   ├── elf.hpp
│   │   │   ├── info.hpp
│   │   │   └── uefi.hpp
│   │   ├── drivers
│   │   │   └── serial.hpp
│   │   └── mm
│   │       └── allocator.hpp
│   └── axiomos
│       └── arch
│           └── x86_64
│               └── apic.hpp
├── LICENSE
├── README.md
├── src
│   ├── bootloader
│   │   └── main.cpp
│   └── kernel
│       ├── arch
│       │   └── x64
│       │       ├── apic.cpp
│       │       ├── cpu.cpp
│       │       ├── entry.S
│       │       ├── gdt.cpp
│       │       ├── gdt.S
│       │       ├── idt.cpp
│       │       └── interrupts.S
│       ├── drivers
│       │   └── serial.cpp
│       ├── linker.ld
│       ├── main.cpp
│       └── mm
│           └── allocator.cpp
├── test_sysv.cpp
├── TODO.md
└── tools
    └── run-qemu.sh
```

## Target Hardware

Primary development target: **HP ProBook 450 G9**
- CPU: 12th Gen Intel Core i3-1215U (Alder Lake, 2P + 4E cores)
- GPU: Intel UHD Graphics (Xe)
- Storage: NVMe SSD
- Firmware: UEFI

See `docs/hardware/` for detailed hardware notes.

## Building from Source

### Prerequisites

- CMake 3.25+
- Clang/LLVM 21+ (recommended: 22+)
- QEMU with OVMF (UEFI) firmware

### Build Steps

```fish
git clone https://github.com/Veridian-Zenith/AxiomOS.git
cd AxiomOS

cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

#### Testing via QEMU

```fish
./tools/run-qemu.sh
```

### Contributing
This is currently a personal learning and experimental project. However, serious contributors are welcome — especially those experienced with modern C++, UEFI, or low-level systems programming.
If you're interested, feel free to open issues or pull requests.

I'm also happy to invite highly interested developers into my private Discord server with appropriate roles.
License
This project is licensed under the OSL-V3 [License](LICENSE).
