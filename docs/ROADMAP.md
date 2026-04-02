# AxiomOS: Strategic Roadmap

This document defines the execution phases for **AxiomOS**, a performance-first, native operating system targeting **12th Gen Intel (Alder Lake)** hardware. Built exclusively with **C++26** and **LLVM 22.1**, the project rejects virtualization bloat in favor of direct hardware authority.

---

## ✅ Phase 1: Minimal Bootable Kernel (COMPLETED)

**Goal:** Establish a functional foundation that boots into a higher-half kernel with serial diagnostics.

* **Toolchain:** Pure LLVM/Clang/LLD pipeline with `-std=c++26`.
* **Bootloader:** UEFI application that loads ELF kernels and constructs `BootInfo`.
* **Memory Transition:** Initial 4GiB identity-mapping to facilitate the jump to the higher-half.
* **Kernel Entry:** Higher-half mapping at `0xFFFFFFFF80000000` with a System V ABI compliant entry point.
* **Diagnostics:** Early UART 16550 serial driver for COM1 logging.

---

## ✅ Phase 2: Architectural Setup (COMPLETED)

**Goal:** Transition from firmware-provided states to kernel-managed CPU and interrupt control.

* **CPU State:** ✅ GDT and TSS initialization logic is implemented.
* **Interrupts:** ✅ IDT is implemented to handle mandatory exceptions.
* **APIC:** ✅ Basic setup for Local APIC and IOAPIC is in place.
* **Memory Purity:** ✅ UEFI mappings are discarded and replaced by PMM.

---

## ✅ Phase 3: Physical Memory Management (PMM) (COMPLETED)

**Goal:** Implement a centralized physical memory allocator to manage raw system RAM.

* **Current Status:** ✅ PMM is implemented, parsing UEFI maps and managing page allocation via a bitmap-based allocator.
* **Map Analysis:** ✅ Parse the UEFI memory map to identify usable RAM versus reserved hardware regions.
* **Bitmap Allocator:** ✅ Implement a high-performance bitmap-based PMM for tracking page availability.
* **Page Primitives:** ✅ Provide `alloc_page()` and `free_page()` as the foundation for higher-level memory layers.

---

## 🔒 Phase 4: Virtual Memory Management (VMM) (NOT STARTED)

**Goal:** Establish a robust virtual memory subsystem for isolation and dynamic mapping.

* **Recursive Paging:** Implement a mechanism for the kernel to modify its own page tables efficiently.
* **Kernel Heap:** Establish a kernel-level heap with `kmalloc` and `kfree` primitives.
* **Dynamic Mapping:** Support on-the-fly mapping for MMIO (Memory Mapped I/O) and hardware regions.

---

## 🧵 Phase 5: Kernel Services & Multitasking (NOT STARTED)

**Goal:** Introduce task switching and hybrid-aware process management.

* **Context Switching:** Implement save/restore logic for CPU registers across multiple tasks.
* **Hybrid Scheduler:** Initial scheduler optimized for the **2 Performance cores** and **4 Efficiency cores** of the i3-1215U.
* **User Mode:** Define the syscall interface and initial user-mode stack architecture.

---

## 💿 Phase 6: Drivers & User Ecosystem (NOT STARTED)

**Goal:** Expand native hardware compatibility and launch the initial user environment.

* **Native Storage:** Implement high-performance **NVMe** support and a native **XFS** primary filesystem driver.
* **I/O Subsystem:** USB 3.0 (xHCI) and Graphics (GOP/Intel Xe) integration.
* **Userspace Foundation:** Launch the first user process and a minimal freestanding C++ standard library.
