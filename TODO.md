# AxiomOS Development Roadmap

> **Core Philosophy:** The system prioritizes resilience over brittleness. For any operation, it will first attempt the expected path. If that fails, it will attempt a set of reasonable alternatives. If the request is illogical, impossible, or potentially malicious, it will be met with a "Strict Denial," which includes a descriptive alert and protective measures.

## Development Strategy

* **Target-First, Abstract-Always:** The kernel architecture must strictly target the hardware documented in [docs/hardware_info.md](./docs/hardware_info.md) for current implementation efficiency.
* **Pluggable Design:** Every hardware-dependent subsystem (memory, interrupts, drivers) must be implemented behind clean interfaces, allowing future hardware abstraction layers (HAL) or device-specific modules to be swapped in without modifying the kernel core.

## Kernel Architecture Choice

* **Design:** A **highly-modular, service-oriented kernel core**.
* **Rationale:** To maximize hardware authority and minimize latency while maintaining unparalleled modularity. By operating in a single address space (avoiding traditional context-switch overhead), we eliminate the bottlenecks of monolithic systems while bypassing the performance hits of microkernel message-passing.
* **Modularity (The Service-Oriented Approach):** This is *not* a standard monolithic kernel. It employs a strict **Service-Oriented Architecture (SOA)** internally. All subsystems (Memory, Interrupts, Drivers, I/O) are designed as independent modules with well-defined APIs and communicate through a central `ServiceRegistry`. This ensures that any part of the system—from memory allocators to hardware drivers—can be swapped or hot-loaded without architectural changes.
* **Performance:** Efficiency is achieved by reducing abstraction layers, not by coupling components. We provide "zero-cost" abstractions through C++26 features, ensuring that the modular structure does not impose runtime performance penalties.

---

## 0️⃣ Phase 0: Project Foundation & Build System

*The goal of this phase is to establish a build environment that can compile our two main components: the UEFI bootloader and the ELF kernel.*

* [x] **1. `CMakeLists.txt` Setup:**
  * [x] Define two primary targets:
    * [x] `bootloader`: Compiles `src/bootloader/**.cpp` using the `x86_64-unknown-uefi` target triple to produce a PE/COFF executable (`.efi`).
    * [x] `kernel`: Compiles `src/kernel/**.cpp` and `src/kernel/**.S` into a freestanding, higher-half ELF64 executable.
  * [x] Enforce strict compiler flags (`-Wall`, `-Wextra`, `-Werror`, `-pedantic`).
  * [x] Set C++ standard to C++26 (`-std=c++26`).
  * [x] Disable features incompatible with freestanding environments (`-fno-exceptions`, `-fno-rtti`, `-nostdlib`, `-fno-stack-protector`).
* [x] **2. `tools/run-qemu.sh` Script:**
  * [x] **Automation:** The script must perform all build, imaging, and execution steps.
  * [x] **Build:** Call CMake/Ninja to build the project.
  * [x] **Imaging:**
    * [x] Create a 64MB FAT32 disk image (`axiom.img`).
    * [x] Create the directory structure `EFI/BOOT/`.
    * [x] Copy `bootloader.efi` to `EFI/BOOT/BOOTX64.EFI`.
    * [x] Copy `kernel.elf` to the root directory.
  * [x] **Execution:** Launch `qemu-system-x86_64` with:
    * [x] Local, user-owned OVMF firmware files.
    * [x] The created disk image.
    * [x] Serial port redirected to `stdio`.
    * [x] **Crucially:** Add `-d int,cpu_reset -no-reboot` flags for detailed fault analysis.

---

## 1️⃣ Phase 1: UEFI Bootloader

*The bootloader is the bridge from firmware to our code. Its only job is to prepare the system and hand off control to the kernel.*

* [x] **1. Entry Point & GOP:**
  * [x] Create `src/bootloader/main.cpp`.
  * [x] Implement `efi_main`.
  * [x] Locate the Graphics Output Protocol (GOP) to get the framebuffer address and screen resolution.
* [x] **2. File Loading:**
  * [x] Implement logic to find and open the `kernel.elf` file from the FAT32 volume.
* [x] **3. ELF Parsing & Loading:**
  * [x] Read the ELF64 header.
  * [x] Iterate through the program headers, find `PT_LOAD` segments.
  * [x] For each segment, allocate physical memory pages using the UEFI `AllocatePages` service and copy the segment data into them.
* [x] **4. Memory Mapping & Handoff:**
  * [x] Get the UEFI memory map.
  * [x] Create a new set of 4-level page tables (PML4).
    * [x] **Identity-map** the first 4GiB of physical memory.
    * [x] **Map the loaded kernel segments** to their higher-half virtual addresses (`0xFFFFFFFF80000000`+).
  * [x] **Construct the `BootInfo` struct:** This C-style struct will contain framebuffer details, UEFI memory map, etc.
  * [x] **`ExitBootServices()`:** The point of no return.
  * [x] Load the new PML4 into the `CR3` register.
  * [x] Jump to the kernel's entry point, passing the address of the `BootInfo` struct.

---

## 2️⃣ Phase 2: Kernel - The First Lines of Code

*This phase establishes a minimal, verifiable kernel environment. Our only tool for debugging is the serial port.*

* [x] **1. Kernel Entry (`entry.S`):**
  * [x] Create `src/kernel/arch/x86_64/entry.S`.
  * [x] Define the `_start` symbol (ELF entry point).
  * [x] **Critical:** Allocate a 16KiB BSS section for the initial kernel stack.
  * [x] Set the `RSP` register to the top of this stack.
  * [x] Move the `BootInfo` pointer from `rcx` (UEFI ABI) to `rdi` (System V ABI).
  * [x] Call the C++ `kmain` function.
* [x] **2. Serial Driver:**
  * [x] Create `src/kernel/utils/serial.cpp` and `.hpp`.
  * [x] Implement simple, polling-based I/O for the COM1 serial port.
  * [x] Provide only three functions: `init()`, `putchar(char)`, and `puts(const char*)`.
  * [x] **No variadic functions (`printf`) will be implemented at this stage.**
* [x] **3. Kernel Main (`kmain`):**
  * [x] Create `src/kernel/main.cpp`.
  * [x] The `kmain` function receives the `BootInfo` pointer.
  * [x] **First Action:** Call `serial::init()`.
  * [x] **Verification:** Print a welcome message and parse `BootInfo`, printing framebuffer/memmap details to the serial console.
  * [x] Enter an infinite `hlt` loop.
  * [x] **Goal:** Successfully boot and see clean, verifiable output in the QEMU console.

---

## 3️⃣ Phase 3: Physical Memory Manager (PMM)

*The PMM is the foundation of all memory management. We will build it according to the Resilient System philosophy.*

* [ ] **1. Design the PMM API and Resilience Policy:**
  * [ ] Create `src/kernel/mm/pmm.hpp`.
  * [ ] **API:** `init(BootInfo*)`, `alloc_pages(count)`, `free_pages(address, count)`, `get_free_memory()`, `get_total_memory()`.
  * [ ] **Resilience Policy (documented in the header):**
    * [ ] **Reasonable Path:** First-fit search from the last allocation point.
    * [ ] **Alternative Paths:**
            1. [ ] If first search fails, restart search from the beginning.
            2. [ ] *(Future Stub)* `alloc_discontiguous_pages()`
            3. [ ] *(Future Stub)* Trigger memory compaction.
            4. [ ] Fail with a specific error code (e.g., `OUT_OF_MEMORY`).
    * [ ] **Strict Denial (Suspicious Acts):**
      * [ ] `alloc_pages(0)`
      * [ ] `free_pages` with a non-page-aligned address.
      * [ ] `free_pages` for memory that is already free (double-free).
      * [ ] `free_pages` for an address outside the managed range.
      * [ ] **Action:** Print a verbose `[PMM-DENIAL]` alert and halt the kernel.
* [ ] **2. Implementation (`allocator.cpp`):**
  * [ ] Implement a bitmap allocator based on the design.
  * [ ] The `init` function will find space for the bitmap and parse the UEFI memory map to populate it.
* [ ] **3. Verification:**
  * [ ] Create `src/kernel/tests/pmm.cpp`.
  * [ ] Implement a `test_pmm()` function called from `kmain`.
  * [ ] The test will:
    * [ ] Allocate and free single pages.
    * [ ] Allocate and free multi-page blocks.
    * [ ] Verify that `get_free_memory()` counts are correct.
    * [ ] **Test all Strict Denial cases** to ensure they are caught and reported correctly.
  * [ ] **Goal:** A successful run of the test suite printed to the serial console, proving PMM reliability.

---

## 4️⃣ Phase 4: Modular VMM and Service Registry

*Establish a pluggable architecture for virtual memory and kernel services.*

* [ ] **1. Paging Interface:**
  * [ ] Define `axiom::mm::PagingInterface` as a C++26 concept/interface for page table management.
  * [ ] Implement `x86_64::PageTableManager` compliant with `PagingInterface`.
* [ ] **2. Service Registry:**
  * [ ] Create `axiom::kernel::ServiceRegistry`.
  * [ ] Allow core services (Memory, Interrupts, Drivers) to register themselves at boot.
  * [ ] Ensure all lookups are O(1) or O(log N) for performance.

---

## 5️⃣ Phase 5: Lightweight Driver Framework

*Drivers must be modular and performant.*

* [ ] **1. Driver Abstraction:**
  * [ ] Define `axiom::drivers::Device` interface.
  * [ ] Implement `serial` driver using this interface.
* [ ] **2. Modular Loading:**
  * [ ] Support dynamic registration of drivers via the Service Registry.
