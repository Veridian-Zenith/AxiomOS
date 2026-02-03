#!/bin/bash

# AxiomOS QEMU Runner Script
# This script runs the AxiomOS UEFI loader in QEMU

cd loader

mkdir -p uefi_disk/EFI/BOOT                                                                                                                                      12:00:26
cp target/x86_64-unknown-uefi/release/axiom_loader.efi uefi_disk/EFI/BOOT/BOOTX64.EFI
fallocate -l 64M axiom_disk.img                                                                                                                                  12:00:26
mkfs.vfat axiom_disk.img
sudo mount -o loop axiom_disk.img /mnt                                                                                                                           12:00:26
sudo cp -r uefi_disk/* /mnt/
sudo umount /mnt

# Run QEMU with UEFI support
qemu-system-x86_64 \
    -enable-kvm \
    -drive file=axiom_disk.img,format=raw \
    -bios /usr/share/OVMF/OVMF_CODE.fd \
    -serial stdio \
    -m 2G \
    -cpu host > log.txt
