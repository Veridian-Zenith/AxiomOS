# System Hardware Information

This document provides detailed hardware specifications for the AxiomOS development and testing environment.

## 1. Processor (CPU)

- **Model**: 12th Gen Intel(R) Core(TM) i3-1215U
- **Architecture**: Alder Lake-UP3 (x86_64)
- **Topology**:
  - **Total Cores**: 6
  - **Total Threads**: 8
  - **P-cores (Performance)**: 2 Cores / 4 Threads (Cores 0-1)
  - **E-cores (Efficiency)**: 4 Cores / 4 Threads (Cores 2-5)
- **Frequencies**:
  - **P-core Max**: 4.40 GHz
  - **E-core Max**: 3.30 GHz (Base listed as 0.9 - 1.2 GHz in power profiles)
  - **Current Scaling**: 400 MHz - 4400 MHz
- **Cache Hierarchy**:
  - **L1d**: 224 KiB (6 instances)
  - **L1i**: 320 KiB (6 instances)
  - **L2**: 4.5 MiB (3 instances - Shared per P-core pair/E-core cluster)
  - **L3**: 10 MiB (1 instance - Shared)
- **Instruction Set Extensions**:
  - AVX, AVX2, AVX-VNNI
  - SHA, AES, VAES, VPCLMULQDQ
  - VT-x (Virtualization)
  - Intel Control-Flow Enforcement Technology (CET): User-mode Shadow Stack (user_shstk), Indirect Branch Tracking (ibt)
- **Microcode**: `0x43b`

## 2. Memory (RAM)

- **Total Capacity**: 16 GiB (Reported as ~15GiB usable)
- **Configuration**: Dual-Channel (2x 8GiB SODIMM)
  - **Slot 1**: 8 GiB DDR4 3200 MT/s (Samsung M471A1K43EB1-CWE)
  - **Slot 2**: 8 GiB DDR4 3200 MT/s (Micron 4ATF1G64HZ-3G2F1)
- **Voltage**: 1.2 V
- **Maximum Supported**: 64 GiB

## 3. Storage

- **Device**: KIOXIA KBG50ZNV256G (NVMe SSD)
- **Controller**: KIOXIA Corporation NVMe SSD Controller BG5 (DRAM-less)
- **Interface**: PCIe G4 x4 (NVM Express 1.4)
- **Capacity**: 256 GB (238.5 GiB)
- **Partitioning**:
  - `nvme0n1p1`: 1 GiB (EFI/System)
  - `nvme0n1p2`: 237.5 GiB (Root/Data)

## 4. Graphics (GPU)

- **Controller**: Intel Corporation Alder Lake-UP3 GT1 [UHD Graphics] (rev 0c)
- **Device Name**: Onboard IGD
- **Memory**:
  - 16 MiB Non-prefetchable BAR
  - 256 MiB Prefetchable BAR
- **Driver**: `xe` (Intel Xe Driver)
- **Features**: GOP (Graphics Output Protocol) Support for UEFI boot.

## 5. Motherboard & Firmware

- **System**: HP ProBook 450 15.6 inch G9 Notebook PC
- **Motherboard**: HP Model 8978 (KBC Version 07.74.00)
- **Firmware (BIOS/UEFI)**:
  - **Vendor**: HP
  - **Version**: U72 Ver. 01.17.00
  - **Release Date**: 11/06/2025
  - **Mode**: UEFI (Secure Boot capable)
  - **SMBIOS**: 3.4
- **PCI ECAM Range**: `[bus 00-ff]`

## 6. Network & Connectivity

- **Wireless**: Realtek RTL8852BE PCIe 802.11ax (Wi-Fi 6)
- **Ethernet**: Realtek RTL8111/8168/8211/8411 Gigabit Ethernet
- **Audio**: Intel Alder Lake PCH-P High Definition Audio (using `sof-audio-pci-intel-tgl`)

## 7. Kernel Boot Parameters (Current)

`initrd=intel-ucode.img initrd=\initramfs-linux-cachyos-eevdf-lto.img root=UUID=... rw nowatchdog quiet splash i915.force_probe=!46b3 xe.force_probe=46b3 intel_iommu=on iommu=pt intel_hfi=on`
