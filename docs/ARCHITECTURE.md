# AxiomOS Architecture

This document provides a high-level overview of the AxiomOS architecture, boot process, and key design decisions.

## Core Principles

- **Monolithic Kernel:** The kernel is designed as a single, large executable running in a single address space. This approach is chosen for simplicity and performance in the early stages.
- **Higher-Half Kernel:** The kernel is linked to run in the higher half of the 64-bit virtual address space (starting at `0xFFFFFFFF80000000`). This separates the kernel's address space from the user's, providing protection and a larger contiguous virtual address space for user processes.
- **64-bit Only:** The OS is exclusively designed for the x86_64 architecture in long mode. Legacy 32-bit support is not a goal.
- **Freestanding C++:** The kernel is built in a freestanding environment, meaning it does not link against a standard C or C++ library and cannot rely on services typically provided by an underlying OS.

## Project Structure

```
AxiomOS/
├── CMakeLists.txt      # Root CMake build system
├── docs/               # Documentation (like this file)
├── include/            # Public headers shared between components
│   └── axiomos/
├── src/
│   ├── bootloader/     # UEFI bootloader source
│   └── kernel/         # Kernel source
│       ├── arch/       # Architecture-specific code (e.g., x86_64)
│       ├── mm/         # Memory management (PMM, VMM)
│       └── utils/      # Utility code (e.g., serial logging)
└── tools/              # Helper scripts for building and running
```

## The Boot Process

The process of starting AxiomOS involves several stages:

1.  **UEFI Firmware:** The platform's firmware initializes the hardware and loads the AxiomOS bootloader (`bootloader.efi`) from a FAT32-formatted EFI System Partition (ESP).

2.  **AxiomOS Bootloader (`bootloader.efi`):**
    - **Execution Environment:** Runs as a 64-bit UEFI application.
    - **Kernel Loading:** Locates the `kernel` file on the same filesystem and loads its ELF segments into physical memory.
    - **Memory Mapping:** Sets up a new set of page tables (PML4) for the kernel. This includes:
        - An identity map of the first 4GB of physical memory to handle the transition and access UEFI services.
        - A higher-half mapping of the kernel's code and data segments to their virtual addresses.
    - **Information Gathering:** Queries UEFI boot services for the system's memory map and graphics output protocol (GOP) for the framebuffer details.
    - **Exiting Boot Services:** Calls `ExitBootServices()` to terminate all UEFI services, rendering them unusable. This is the point of no return.
    - **Handoff:** Loads the new PML4 into the `CR3` register, and jumps to the kernel's entry point, passing a `BootInfo` struct containing the memory map and framebuffer information.

3.  **AxiomOS Kernel:**
    - **Entry Point (`entry.S`):** A small assembly stub receives the `BootInfo` struct (adjusting the calling convention from UEFI to System V) and calls the C++ kernel main function (`kmain`).
    - **Initialization (`kmain`):**
        1.  **Serial Logging:** Initializes the COM1 serial port for debug output.
        2.  **CPU Features:** Detects CPU vendor and features using `CPUID`.
        3.  **Core Architecture:** Sets up the Global Descriptor Table (GDT) and a basic Interrupt Descriptor Table (IDT). The legacy PIC is also remapped and disabled to prepare for APIC.
        4.  **Memory Management:**
            - The **Physical Memory Manager (PMM)** is initialized using the UEFI memory map to track available physical pages.
            - The **Virtual Memory Manager (VMM)** takes control of the page tables created by the bootloader.
        5.  **Idle State:** After initialization, the kernel enters an infinite `hlt` loop, as there is no scheduler or userland to run yet.

## Memory Layout

- **Virtual Address Space:** A 48-bit virtual address space is used.
    - `0x0000000000000000` - `0x0000FFFFFFFFFFFF`: User space (lower half)
    - `0xFFFF800000000000` - `0xFFFFFFFFFFFFFFFF`: Kernel space (higher half, canonical)
- **Physical Memory:** The PMM uses a bitmap to track the usage of all available physical memory pages reported by UEFI.

## Future Work (High-Level Roadmap)

1.  **Interrupts & Exceptions:** Fully implement the IDT with handlers for CPU exceptions and hardware interrupts.
2.  **APIC & Timers:** Initialize the local APIC (and IOAPIC) to handle interrupts from modern hardware and provide a reliable system timer.
3.  **Scheduler:** Implement a basic scheduler to run multiple tasks.
4.  **ACPI:** Parse ACPI tables to discover hardware, especially for multi-core startup.
5.  **Syscalls:** Define a system call interface for userland processes.
6.  **Userland:** Load and run the first user-space process.
7.  **Drivers:** Begin writing drivers for essential hardware like storage (NVMe) and graphics (basic framebuffer).
