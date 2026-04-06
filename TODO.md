# AxiomOS Development TODO

## 0️⃣ Phase 0: The Axiom Standard (The Contract)

*Establish the foundational constraints and interface patterns that guarantee modularity and performance.*

- [ ] **0.1 Contract Headers (.hpp)**
  - [ ] Define the header-only contract standard for all kernel-level interfaces.
  - [ ] Enforce zero-dependency inclusion for contract headers.
- [ ] **0.2 State Sovereignty**
  - [ ] Audit and remove any potential global state from core components.
  - [ ] Implement explicit context passing for all service interactions.
- [ ] **0.3 Execution Strategy**
  - [ ] **Standalone Builds:** Enable per-module compilation and unit testing.
  - [ ] **Linux-based Test Harness:** Develop a mock environment for rapid logic validation.
  - [ ] **Hardware Deployment:** Finalize the pipeline for bare-metal HP ProBook 450 G9 flashing.

---

## 1️⃣ Phase 1: The "Sentinel" (UEFI Boot-Bridge)

*The Sentinel establishes the bridge from firmware to our code, ensuring a secure and known environment.*

- [ ] **1.1 Project Foundation & Toolchain**
  - [ ] Configure `CMakeLists.txt` to enforce `clang-22` and `-std=c++26`.
  - [ ] Implement freestanding environment flags (`-ffreestanding`, `-fno-exceptions`, `-fno-rtti`).
  - [ ] Create `tools/run-qemu.sh` for UEFI-aware disk image synthesis.
- [ ] **1.2 UEFI Entry & Graphics**
  - [ ] Implement `efi_main` entry point with error-handling for `EFI_SYSTEM_TABLE`.
  - [ ] Query and select optimal resolution via Graphics Output Protocol (GOP).
  - [ ] Implement early screen logging (fallback for serial).
- [ ] **1.3 ELF64 Kernel Loader**
  - [ ] Implement robust ELF64 header validation (Magic, Type, Machine).
  - [ ] Map `PT_LOAD` segments into higher-half virtual memory (`0xFFFFFFFF80000000`).
  - [ ] Zero-fill `.bss` section in accordance with program headers.
- [ ] **1.4 Secure Handoff**
  - [ ] Construct `BootInfo` structure containing UEFI Memory Map and ACPI pointers.
  - [ ] Implement `ExitBootServices` with map-integrity verification.
  - [ ] Jump to kernel `_start` with `%rdi` pointing to `BootInfo`.

---

## 2️⃣ Phase 2: The "Registry" (Micro-Monolithic Kernel)

*The Registry core manages CPU, interrupts, and memory as high-performance services.*

- [ ] **2.1 Kernel Entry & Early Diagnostics**
  - [ ] Define `_start` in `src/kernel/arch/x64/entry.S`.
  - [ ] Establish initial 16KiB kernel stack and early GDT.
  - [ ] Initialize polling-based UART 16550 driver for debug output.
- [ ] **2.2 Memory Sovereignty**
  - [ ] **Physical Memory Manager:** Implement bitmap-based allocator managing 14GiB RAM.
  - [ ] **Virtual Memory Manager:** Implement recursive paging and 4-level PML4 management.
  - [ ] **Kernel Heap:** Implement C++26 compatible Slab/Buddy allocator for `kmalloc`.
- [ ] **2.3 Architectural Core**
  - [ ] Initialize IDT (Interrupt Descriptor Table) and basic handlers.
  - [ ] Configure Local APIC and I/O APIC for the Alder Lake SoC.
  - [ ] Implement SMP (Symmetric Multi-Processing) for P-cores and E-cores.
- [ ] **2.4 Service Registry & ID:0 Authority**
  - [ ] Implement `axiom::kernel::ServiceRegistry` for lock-free service lookups.
  - [ ] Implement Ed25519-based signature verification for all kernel modules.
  - [ ] Enforce "ID:0" authority for all memory-mapping requests.

---

## 3️⃣ Phase 3: The "Plug-in" I/O (User-Space Drivers)

*Move hardware interaction into isolated, modular services.*

- [ ] **3.1 PCIe & Bus Discovery**
  - [ ] Scan MCFG table to locate PCIe configuration space.
  - [ ] Enumerate Alder Lake PCH devices and identify KIOXIA NVMe/Intel UHD Graphics.
- [ ] **3.2 Driver Isolation Framework**
  - [ ] Implement Ring 3 process creation for drivers.
  - [ ] Define Shared-Memory IPC (SM-IPC) for zero-copy data transfer.
  - [ ] Map MMIO BARs directly into driver address spaces with kernel-mediated protection.
- [ ] **3.3 Primary Driver Set (Port & Adapt Strategy)**
  - [ ] Implement xHCI driver for USB 3.2 support.
  - [ ] **Ported Graphics:** Adapt Intel `xe` driver logic for basic Alder Lake UHD Graphics support.
  - [ ] **Ported Audio:** Adapt Intel `sof` (Sound Open Firmware) for audio I/O initialization.
  - [ ] **Firmware Integration:** Implement a loader for legally redistributable firmware binaries (DSP/GPU).
- [ ] **3.4 Native Transition (Long-term)**
  - [ ] Refactor ported xHCI into native Plug-in I/O module.
  - [ ] Implement native UHD Graphics framebuffer control, replacing ported components.

---

## 4️⃣ Phase 4: The "Axiom-VFS" (3-Boot Storage)

*High-speed storage and filesystem orchestration.*

- [ ] **4.1 NVMe Native Stack**
  - [ ] Implement native NVMe controller initialization for KIOXIA BG5.
  - [ ] Develop high-priority Submission/Completion queue management.
- [ ] **4.2 Filesystem Hierarchy**
  - [ ] **Boot 1:** Implement FAT32 read support for UEFI partition.
  - [ ] **Boot 2:** Implement high-integrity read-only loader for the core kernel.
  - [ ] **Boot 3:** Implement full XFS driver for primary system partition.
- [ ] **4.3 VFS Features**
  - [ ] Implement "Pinning System" for permanent memory-mapping of critical binaries.
  - [ ] Develop HighwayHash-based data integrity verification for all reads.

---

## 5️⃣ Phase 5: The "Fish" Shell

*The user environment and system interaction.*

- [ ] **5.1 Userspace Foundation**
  - [ ] Define System V ABI compliant syscall interface.
  - [ ] Implement process management and signal handling.
- [ ] **5.2 Hardware-Accelerated UI**
  - [ ] Develop native DRM/KMS-style interface for Intel UHD Graphics.
  - [ ] Implement low-latency terminal rendering pipeline.
- [ ] **5.3 The Fish Shell Implementation**
  - [ ] Build C++26 optimized CLI with real-time hardware telemetry overlays.
  - [ ] Implement signed-execution enforcement in the shell's process launcher.
