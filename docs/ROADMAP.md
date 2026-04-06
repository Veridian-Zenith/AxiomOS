# AxiomOS Architectural Roadmap (v1.0)

This document defines the strategic execution phases for **AxiomOS**, a high-performance, native operating system targeting the **Intel Core i3-1215U** (Alder Lake) architecture. AxiomOS leverages **C++26** and **LLVM** to exercise direct hardware authority, optimizing for the unique hybrid core topology (2 P-cores, 4 E-cores) of the target platform.

---

## Phase 1: The "Sentinel" (UEFI Boot-Bridge) [PLANNED]

**Goal:** Establish a secure, high-integrity handoff from UEFI firmware to the AxiomOS kernel.

- **UEFI Runtime Integration:** Leverages UEFI services for initial hardware discovery and memory map acquisition.
- **ELF64 Loader:** Robust parsing of the kernel executable, ensuring correct segment alignment and protection attributes.
- **Higher-Half Transition:** Establishes the initial Paging (PML4) to map the kernel into the `0xFFFFFFFF80000000` virtual address space.
- **Boot Diagnostics:** Early serial output (UART 16550) for real-time boot tracking.

## Phase 2: The "Registry" (Micro-Monolithic Kernel) [PLANNED]

**Goal:** A service-oriented kernel core that manages hardware resources with zero-cost abstractions.

- **Hybrid-Aware Scheduling:** Optimized task distribution across Intel's Performance (P-cores) and Efficiency (E-cores) using the Intel Thread Director insights where applicable.
- **Memory Sovereignty:**
  - **PMM (Physical Memory Manager):** Bitmap-based allocation managing the 14GiB system RAM.
  - **VMM (Virtual Memory Manager):** Recursive paging and kernel heap (Slab/Buddy) implementation.
- **Interrupt Architecture:** Low-latency IDT and APIC (Local/IO) configuration for precise hardware event handling.
- **Service Registry:** A central, high-speed lookup for kernel-level services, facilitating a modular but single-address-space design.

## Phase 3: The "Plug-in" I/O (User-Space Drivers) [PLANNED]

**Goal:** Shift hardware drivers out of the core kernel to enhance system resilience.

- **Driver Isolation:** Drivers run in isolated contexts, communicating via the Service Registry.
- **PCIe Enumeration:** Native discovery and management of the Alder Lake PCH components.
- **USB 3.2 xHCI:** High-speed I/O stack for external peripherals.

## Phase 4: The "Axiom-VFS" (3-Boot Storage) [PLANNED]

**Goal:** A high-performance virtual filesystem supporting multi-boot and native storage.

- **NVMe Native Driver:** Direct command submission to the KIOXIA NVMe controller.
- **XFS Support:** Read/Write support for the primary XFS partition.
- **3-Boot Logic:** Orchestrating the relationship between UEFI/FAT32 boot partitions and the main OS storage.

## Phase 5: The "Fish" Shell [PLANNED]

**Goal:** A modern, ergonomics-focused user interface and command-line environment.

- **System V ABI Userspace:** Full support for standard C++26 userspace applications.
- **Graphics Output:** Utilizing the Intel UHD Graphics (64 EUs) via GOP and eventually native DRM/KMS.
- **The Fish Shell:** A performance-tuned, user-centric shell for interacting with the AxiomOS ecosystem.

---
**Core Technology Stack:**

- **Language:** C++26 (Freestanding)
- **Compiler:** LLVM / Clang 22.1
- **Architecture:** x86_64 (Intel Alder Lake)
- **Memory Model:** Pure 64-bit, Higher-Half Kernel
