# AxiomOS

A modern, C++23, 64-bit operating system for x86_64 UEFI platforms, built from scratch.

## Project Goals

- **Modern C++:** Leverage C++23 features for a safer, more expressive kernel and user space.
- **Security-First:** Implement modern security mitigations and best practices from the ground up.
- **LLVM-Native:** Use the Clang/LLVM toolchain for its advanced diagnostics, performance, and modern features.
- **Custom Tooling:** Develop a custom package manager and other essential utilities tailored for AxiomOS.
- **Clean and Documented:** Maintain high code quality standards with thorough documentation.

## Current Status

The project is in its early stages. The current codebase provides:

- A UEFI bootloader that can load and execute the kernel.
- A higher-half kernel mapped to `0xFFFFFFFF80000000`.
- Basic hardware initialization (GDT, IDT, PIC).
- A physical memory manager (bitmap allocator).
- Serial logging for debugging.
- CPU feature detection.

The system currently boots to a serial console and prints system information before halting.

## Target Hardware

The primary development target is an **HP ProBook 450 G9** with the following specifications:

- **CPU:** 12th Gen Intel Core i3-1215U (Alder Lake, 2P+4E cores)
- **GPU:** Intel UHD Graphics (Xe)
- **Storage:** NVMe SSD
- **Firmware:** UEFI

While the project targets this specific hardware, the goal is to remain as platform-independent as possible where it doesn't conflict with performance or feature goals.

## Building from Source

### Prerequisites

- **CMake** (version 3.25+)
- **Clang/LLVM** toolchain (version 15+)
- **QEMU** for emulation (with OVMF firmware for UEFI)

### Build Steps

1. **Clone the repository:**

    ```sh
    git clone https://github.com/your-username/AxiomOS.git
    cd AxiomOS
    ```

2. **Configure the build:**

    ```sh
    cmake -B build
    ```

3. **Compile the code:**

    ```sh
    cmake --build build
    ```

The final bootable EFI file (`bootloader.efi`) and the kernel (`kernel`) will be located in the `build/` directory.

## Running in QEMU

A convenience script for running the OS in QEMU will be added soon. In the meantime, you can manually create a disk image and run it:

```sh
# (Commands to create a FAT32 image, copy bootloader.efi and kernel...)

qemu-system-x86_64 \
  -bios /path/to/OVMF_CODE.fd \
  -drive format=raw,file=your_image.img \
  -m 2G \
  -serial stdio
```

## Contributing

This is a personal learning project, but contributions and suggestions are welcome. Please adhere to the [Coding Standards](docs/CODING_STANDARDS.md) when submitting pull requests.

## License

This project is licensed under the [MIT License](LICENSE).
