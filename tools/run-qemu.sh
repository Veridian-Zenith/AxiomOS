#!/bin/bash
set -e

# If a log file is provided, redirect all output to it
if [ -n "$1" ]; then
    exec &> "$1"
fi

# Configuration
BUILD_DIR="build"
IMAGE_NAME="axiom.img"
OVMF_CODE="firmware/OVMF_CODE.4m.fd"
OVMF_VARS="firmware/OVMF_VARS.4m.fd"

echo "[BUILD] Configuring and Compiling..."
# Explicitly use Ninja and the build directory
cmake -B $BUILD_DIR -G Ninja
cmake --build $BUILD_DIR

echo "[IMAGE] Creating 64MB GPT Disk Image..."
# Create a blank 64MB file
dd if=/dev/zero of=$IMAGE_NAME bs=1M count=64

# Create GPT partition table and an EFI System Partition (ESP)
# Modern UEFI (Alder Lake) requires GPT for NVMe boot
parted $IMAGE_NAME -s mklabel gpt
parted $IMAGE_NAME -s mkpart ESP fat32 1MiB 100%
parted $IMAGE_NAME -s set 1 esp on

echo "[IMAGE] Mapping and Formatting Partition..."
# Use loop device with partition scanning to bypass mtools issues
LOOP_DEV=$(sudo losetup --show -fP "$IMAGE_NAME")

# Ensure cleanup happens even if the script fails
cleanup() {
    echo "[CLEANUP] Detaching $LOOP_DEV..."
    sudo losetup -d "$LOOP_DEV" || true
}
trap cleanup EXIT

# Format the first partition (p1) as FAT32
sudo mkfs.vfat -F 32 "${LOOP_DEV}p1"

echo "[IMAGE] Copying bootloader and kernel..."
MOUNT_DIR=$(mktemp -d)
sudo mount "${LOOP_DEV}p1" "$MOUNT_DIR"

# Standard UEFI boot path: /EFI/BOOT/BOOTX64.EFI
sudo mkdir -p "$MOUNT_DIR/EFI/BOOT"
sudo cp "$BUILD_DIR/bootloader.efi" "$MOUNT_DIR/EFI/BOOT/BOOTX64.EFI"
sudo cp "$BUILD_DIR/kernel" "$MOUNT_DIR/kernel"

sudo umount "$MOUNT_DIR"
rmdir "$MOUNT_DIR"

echo "[QEMU] Launching AxiomOS (NVMe Target)..."
# Using -cpu host and -accel kvm for native performance on i3-1215U
qemu-system-x86_64 \
    -machine q35 \
    -cpu host \
    -accel kvm \
    -m 2G \
    -drive if=pflash,format=raw,readonly=on,file=$OVMF_CODE \
    -drive if=pflash,format=raw,file=$OVMF_VARS \
    -drive file=$IMAGE_NAME,if=none,id=nvm,format=raw \
    -device nvme,serial=axiom-01,drive=nvm,bootindex=0 \
    -boot menu=on,splash-time=0 \
    -nographic \
    -no-reboot \
    -d int,cpu_reset,guest_errors
