# AxiomOS Development Roadmap

This document outlines the strategic phases for building AxiomOS, a modern, resilient operating system targeting 12th Gen Intel (Alder Lake) hardware and utilizing C++26.

## Phase 1: Minimal Bootable Kernel (Completed)

**Goal:** Establish a functional, C++26-compliant foundation that boots into a higher-half kernel and provides serial output.

* **Build System:** Pure LLVM/Clang/LLD toolchain with C++26 enabled (`-std=c++26`).
* **Bootloader:** UEFI application that loads ELF kernels, constructs `BootInfo`, and identity-maps the lower 4GiB for transition.
* **Kernel Foundation:**
  * Higher-half mapping at `0xFFFFFFFF80000000`.
  * Assembly entry point (`entry.S`) for System V ABI transition.
  * Early UART 16550 serial driver for COM1 logging.
* **Automation:** `run-qemu.sh` for integrated build-image-run workflows.

---

## Phase 2: Architectural Setup (GDT/IDT/APIC)

**Goal:** Transition from firmware-provided settings to safe, kernel-managed CPU and interrupt states.

* **GDT/TSS:** Initialize the Global Descriptor Table and Task State Segment for x86_64.
* **IDT:** Implement the Interrupt Descriptor Table and handle mandatory hardware exceptions (Page Faults, GPF, etc.).
* **APIC:** Setup the Local APIC and IOAPIC for modern interrupt management.
* **Memory Purity:** Transition to kernel-owned page tables, discarding the temporary UEFI identity mappings.

---

## Phase 3: Physical Memory Management (PMM)

**Goal:** Implement a centralized physical memory allocator to manage system RAM.

* **Memory Map Analysis:** Parse the UEFI memory map for usable RAM and reserved regions.
* **Bitmap Allocator:** Implement a high-performance bitmap-based PMM.
* **Page Primitives:** Provide `alloc_page()` and `free_page()` for subsequent memory layers.

---

## Phase 4: Virtual Memory Management (VMM)

**Goal:** Establish a higher-half virtual memory subsystem for kernel and user-space isolation.

* **Recursive Paging:** Implement a robust mechanism for modifying page tables.
* **Kernel Heap:** Establish a kernel-level heap with `kmalloc` / `kfree`.
* **Dynamic Mapping:** Support on-the-fly mapping of hardware MMIO and memory regions.

---

## Phase 5: Kernel Services & Multitasking

**Goal:** Introduce task switching and basic process management.

* **Context Switching:** Save and restore CPU states for multiple tasks.
* **Prototypical Scheduler:** Initial round-robin scheduler for kernel threads.
* **User Mode Preparation:** Define the system call interface and initial user-mode stack setup.

---

## Phase 6: Drivers & User Ecosystem (Future)

**Goal:** Expand hardware compatibility and establish the user-facing operating system.

* **Storage Subsystem:** NVMe/AHCI support for persistent data.
* **Input/Output:** USB 3.0 (xHCI) and Graphics (GOP) integration.
* **Userspace Foundation:** Implementation of the initial user process and a minimal C++ standard library.
