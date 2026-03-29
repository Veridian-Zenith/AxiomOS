#!/bin/bash
set -e

# Configuration
BUILD_DIR="build"
IMAGE_NAME="axiom.img"
OVMF_CODE="firmware/OVMF_CODE.fd"
OVMF_VARS="firmware/OVMF_VARS.fd"

echo "[BUILD] Configuring with CMake..."
cmake -B $BUILD_DIR -G Ninja

echo "[BUILD] Compiling..."
cmake --build $BUILD_DIR

echo "[IMAGE] Creating 64MB FAT32 disk image..."
dd if=/dev/zero of=$IMAGE_NAME bs=1M count=64
mkfs.vfat -F 32 $IMAGE_NAME

echo "[IMAGE] Copying bootloader and kernel..."
mmd -i $IMAGE_NAME ::/EFI
mmd -i $IMAGE_NAME ::/EFI/BOOT
mcopy -i $IMAGE_NAME $BUILD_DIR/bootloader.efi ::/EFI/BOOT/BOOTX64.EFI
mcopy -i $IMAGE_NAME $BUILD_DIR/kernel ::/kernel

echo "[QEMU] Launching AxiomOS..."
qemu-system-x86_64 \
    -drive if=pflash,format=raw,readonly=on,file=$OVMF_CODE \
    -drive if=pflash,format=raw,file=$OVMF_VARS \
    -drive format=raw,file=$IMAGE_NAME \
    -m 2G \
    -nographic \
    -serial mon:stdio \
    -no-reboot \
    -no-shutdown \
    -d int,cpu_reset,guest_errors -no-reboot -no-shutdown
