# Registry Specification (Micro-Monolithic Kernel)

## 1. Overview

The Registry is the AxiomOS kernel core. It adopts a "Micro-Monolithic" architecture: it runs in a single address space (Ring 0) for maximum performance but enforces strict modularity and resource sovereignty through a central service registry and digital signature authority.

## 2. Memory Sovereignty

### 2.1 PMM (Physical Memory Manager)

- **Mechanism:** Bitmap-based allocation managing the full 14GiB system RAM.
- **Optimization:** NUMA-aware (where applicable) and optimized for 4KiB, 2MiB (Large), and 1GiB (Huge) page sizes.
- **Sovereignty:** Tracks ownership of physical frames to prevent unauthorized access across services.

### 2.2 VMM (Virtual Memory Manager)

- **Layout:** Higher-half kernel (`0xFFFFFFFF80000000` to `-1`). All kernel-mode code and structures reside in this 2GiB window.
- **Paging:** 4-level paging (PML4) with recursive mapping at the 510th entry.
- **Service Registry Address:** The global lock-free service directory is located at a fixed virtual address: `0xFFFFFFFF80000000`.

### 2.3 Kernel Heap

- **Slab Allocator:** For small, fixed-size kernel objects (e.g., thread descriptors, mutexes).
- **Buddy Allocator:** For large, variable-size allocations (e.g., DMA buffers).

## 3. Hybrid-Affinity Scheduler

Optimized for the Intel Core i3-1215U (2 P-cores, 4 E-cores).

- **Core Tagging:** Each core is identified by its performance profile (Performance vs. Efficiency).
- **Task Affinity:**
  - **P-Cores:** Assigned high-throughput, low-latency, or compute-heavy threads (e.g., JIT compilers, active UI threads).
  - **E-Cores:** Assigned background tasks, telemetry, and low-priority I/O.
- **Intel Thread Director (ITD):** Utilizes hardware feedback (via MSRs) to dynamically rebalance threads based on thermal and performance constraints.

## 4. ID:0 Authority (Digital Signatures)

The kernel core acts as the ultimate trust root ("ID:0").

- **Signed Modules:** Every kernel module, driver, or critical service must be digitally signed (Ed25519).
- **Integrity Check:** The Registry validates signatures before loading any code into the address space.
- **Revocation:** Maintains a block-list of revoked signatures to mitigate compromised components.
- **Enforcement:** Unsigned code execution is strictly prohibited by the CPU's MBEC (Mode-Based Execute Control) where supported.

## 5. Service Registry

A high-speed, lock-free directory of all system services.

- **Lookups:** Services are located by name or UUID, returning a direct-call address or a shared-memory IPC handle.
- **Modularity:** Services can be registered, updated, or removed dynamically without a full kernel reboot (provided they are not core dependencies).
- **Zero-Copy:** Facilitates direct data transfer between services within the kernel address space.

## 6. Interrupt Architecture

- **APIC (Advanced Programmable Interrupt Controller):**
  - **Local APIC:** One per core for local timers and IPIs.
  - **I/O APIC:** Manages external hardware interrupts.
- **MSI-X:** Preferred interrupt mechanism for high-performance PCIe devices (like NVMe).
- **Latency Management:** Minimizes interrupt disable time through fine-grained locking and deferred procedure calls (DPCs).
