# Building AxiomOS

This guide provides the necessary steps to compile and run AxiomOS from source.

## 1. Prerequisites

You will need a modern Linux environment with the following tools installed:

- **Git:** For cloning the repository.
- **CMake:** Version 3.25 or newer.
- **Clang/LLVM:** Version 15 or newer. The build system relies on `clang`, `lld`, and other LLVM tools.
- **QEMU:** For running the OS in an emulator. You will also need the `OVMF` firmware package for UEFI support (e.g., `ovmf` on Arch Linux, `ovmf` on Debian/Ubuntu).

## 2. Getting the Source

First, clone the AxiomOS repository to your local machine:

```sh
git clone https://github.com/your-username/AxiomOS.git
cd AxiomOS
```

## 3. Compilation

The project uses a standard CMake workflow.

1.  **Configure the build directory:**
    ```sh
    cmake -B build -G Ninja
    ```
    *(Note: `-G Ninja` is optional but recommended for faster builds.)*

2.  **Compile the source code:**
    ```sh
    cmake --build build
    ```

After the build completes, the bootloader (`bootloader.efi`) and the kernel (`kernel`) will be located in the `build/` directory.

## 4. Running in QEMU

To run AxiomOS, you need to create a FAT32 disk image and place the bootloader and kernel on it according to the UEFI specification.

### Automated Script (Recommended)

A helper script is provided to automate this process.

```sh
# (To be created)
./tools/run-qemu.sh
```

### Manual Setup

If you wish to run QEMU manually:

1.  **Create a disk image:**
    ```sh
    dd if=/dev/zero of=axiom.img bs=1M count=64
    mkfs.fat -F32 axiom.img
    ```

2.  **Mount the image and copy files:**
    ```sh
    # Using mtools (safe)
    mcopy -i axiom.img build/kernel ::/
    mmd -i axiom.img ::/EFI
    mmd -i axiom.img ::/EFI/BOOT
    mcopy -i axiom.img build/bootloader.efi ::/EFI/BOOT/BOOTX64.EFI
    ```
    *or, using a loop device (requires root):*
    ```sh
    sudo mount -o loop axiom.img /mnt
    sudo mkdir -p /mnt/EFI/BOOT
    sudo cp build/kernel /mnt/
    sudo cp build/bootloader.efi /mnt/EFI/BOOT/BOOTX64.EFI
    sudo umount /mnt
    ```

3.  **Launch QEMU:**
    Find your `OVMF_CODE.fd` file (often in `/usr/share/ovmf/` or `/usr/share/edk2-ovmf/`).
    ```sh
    qemu-system-x86_64 \
      -bios /path/to/OVMF_CODE.fd \
      -drive format=raw,file=axiom.img \
      -m 2G \
      -cpu host \
      -serial stdio
    ```

## 5. Troubleshooting

- **`clang++: error: invalid target 'x86_64-unknown-uefi'`**
  - Your version of Clang is too old. Please upgrade to at least version 15.

- **QEMU exits immediately or shows a black screen**
  - Ensure `OVMF_CODE.fd` is the correct path.
  - Verify that the bootloader was correctly copied to `/EFI/BOOT/BOOTX64.EFI` on the image.
  - Check the serial output for any early error messages from the bootloader or kernel.
