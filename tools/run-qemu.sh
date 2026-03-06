#!/bin/bash
#
# Helper script to build AxiomOS, create a disk image, and run it in QEMU.
#

set -e

# 1. Build the OS
echo "Building AxiomOS..."
cmake -B build -G Ninja
cmake --build build

# 2. Create Disk Image
IMAGE_FILE="build/axiom.img"
IMAGE_SIZE_MB=64

echo "Creating a ${IMAGE_SIZE_MB}MB disk image at ${IMAGE_FILE}..."
dd if=/dev/zero of="${IMAGE_FILE}" bs=1M count=${IMAGE_SIZE_MB}
mkfs.fat -F32 "${IMAGE_FILE}"

# 3. Copy Files to Image
echo "Copying files to the disk image..."
mcopy -i "${IMAGE_FILE}" build/kernel ::/
mmd -i "${IMAGE_FILE}" ::/EFI
mmd -i "${IMAGE_FILE}" ::/EFI/BOOT
mcopy -i "${IMAGE_FILE}" build/bootloader.efi ::/EFI/BOOT/BOOTX64.EFI

# 4. Find OVMF Firmware
# Common paths for OVMF firmware file
OVMF_PATHS=(
    "/usr/share/ovmf/OVMF_CODE.fd"
    "/usr/share/edk2-ovmf/x64/OVMF_CODE.fd"
    "/usr/share/OVMF/OVMF_CODE.fd"
)

OVMF_FILE=""
for path in "${OVMF_PATHS[@]}"; do
    if [ -f "$path" ]; then
        OVMF_FILE="$path"
        break
    fi
done

if [ -z "$OVMF_FILE" ]; then
    echo "Error: Could not find OVMF_CODE.fd. Please install OVMF/EDK2 for UEFI support."
    exit 1
fi

# 5. Run QEMU
echo "Starting QEMU..."
qemu-system-x86_64 \
  -bios "${OVMF_FILE}" \
  -drive format=raw,file="${IMAGE_FILE}" \
  -m 2G \
  -cpu host \
  -serial stdio
