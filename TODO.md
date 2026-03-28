# AxiomOS Development Roadmap

> **Core Philosophy:** The system prioritizes resilience over brittleness. For any operation, it will first attempt the expected path. If that fails, it will attempt a set of reasonable alternatives. If the request is illogical, impossible, or potentially malicious, it will be met with a "Strict Denial," which includes a descriptive alert and protective measures.

## Technical Stack

* **Architecture:** `x86_64`
* **CPU Mode:** 64-bit Long Mode
* **Firmware:** UEFI Class 3 (no legacy BIOS/CSM)
* **Language:** C++26 (`-std=c++26`)
* **Toolchain:** LLVM/Clang/LLD only (Linux host)
* **Kernel:** Higher-Half, linked at `0xFFFFFFFF80000000`
* **Build System:** CMake + Ninja

---

## 0️⃣ Phase 0: Project Foundation & Build System

*The goal of this phase is to establish a build environment that can compile our two main components: the UEFI bootloader and the ELF kernel.*

* [ ] **1. `CMakeLists.txt` Setup:**
  * [ ] Define two primary targets:
    * [ ] `bootloader`: Compiles `src/bootloader/**.cpp` using the `x86_64-unknown-uefi` target triple to produce a PE/COFF executable (`.efi`).
    * [ ] `kernel`: Compiles `src/kernel/**.cpp` and `src/kernel/**.S` into a freestanding, higher-half ELF64 executable.
  * [ ] Enforce strict compiler flags (`-Wall`, `-Wextra`, `-Werror`, `-pedantic`).
  * [ ] Set C++ standard to C++26 (`-std=c++26`).
  * [ ] Disable features incompatible with freestanding environments (`-fno-exceptions`, `-fno-rtti`, `-nostdlib`, `-fno-stack-protector`).
* [ ] **2. `tools/run-qemu.sh` Script:**
  * [ ] **Automation:** The script must perform all build, imaging, and execution steps.
  * [ ] **Build:** Call CMake/Ninja to build the project.
  * [ ] **Imaging:**
    * [ ] Create a 64MB FAT32 disk image (`axiom.img`).
    * [ ] Create the directory structure `EFI/BOOT/`.
    * [ ] Copy `bootloader.efi` to `EFI/BOOT/BOOTX64.EFI`.
    * [ ] Copy `kernel.elf` to the root directory.
  * [ ] **Execution:** Launch `qemu-system-x86_64` with:
    * [ ] Local, user-owned OVMF firmware files.
    * [ ] The created disk image.
    * [ ] Serial port redirected to `stdio`.
    * [ ] **Crucially:** Add `-d int,cpu_reset -no-reboot` flags for detailed fault analysis.

---

## 1️⃣ Phase 1: UEFI Bootloader

*The bootloader is the bridge from firmware to our code. Its only job is to prepare the system and hand off control to the kernel.*

* [ ] **1. Entry Point & GOP:**
  * [ ] Create `src/bootloader/main.cpp`.
  * [ ] Implement `efi_main`.
  * [ ] Locate the Graphics Output Protocol (GOP) to get the framebuffer address and screen resolution.
* [ ] **2. File Loading:**
  * [ ] Implement logic to find and open the `kernel.elf` file from the FAT32 volume.
* [ ] **3. ELF Parsing & Loading:**
  * [ ] Read the ELF64 header.
  * [ ] Iterate through the program headers, find `PT_LOAD` segments.
  * [ ] For each segment, allocate physical memory pages using the UEFI `AllocatePages` service and copy the segment data into them.
* [ ] **4. Memory Mapping & Handoff:**
  * [ ] Get the UEFI memory map.
  * [ ] Create a new set of 4-level page tables (PML4).
    * [ ] **Identity-map** the first 4GiB of physical memory.
    * [ ] **Map the loaded kernel segments** to their higher-half virtual addresses (`0xFFFFFFFF80000000`+).
  * [ ] **Construct the `BootInfo` struct:** This C-style struct will contain framebuffer details, UEFI memory map, etc.
  * [ ] **`ExitBootServices()`:** The point of no return.
  * [ ] Load the new PML4 into the `CR3` register.
  * [ ] Jump to the kernel's entry point, passing the address of the `BootInfo` struct.

---

## 2️⃣ Phase 2: Kernel - The First Lines of Code

*This phase establishes a minimal, verifiable kernel environment. Our only tool for debugging is the serial port.*

* [ ] **1. Kernel Entry (`entry.S`):**
  * [ ] Create `src/kernel/arch/x86_64/entry.S`.
  * [ ] Define the `_start` symbol (ELF entry point).
  * [ ] **Critical:** Allocate a 16KiB BSS section for the initial kernel stack.
  * [ ] Set the `RSP` register to the top of this stack.
  * [ ] Move the `BootInfo` pointer from `rcx` (UEFI ABI) to `rdi` (System V ABI).
  * [ ] Call the C++ `kmain` function.
* [ ] **2. Serial Driver:**
  * [ ] Create `src/kernel/utils/serial.cpp` and `.hpp`.
  * [ ] Implement simple, polling-based I/O for the COM1 serial port.
  * [ ] Provide only three functions: `init()`, `putchar(char)`, and `puts(const char*)`.
  * [ ] **No variadic functions (`printf`) will be implemented at this stage.**
* [ ] **3. Kernel Main (`kmain`):**
  * [ ] Create `src/kernel/main.cpp`.
  * [ ] The `kmain` function receives the `BootInfo` pointer.
  * [ ] **First Action:** Call `serial::init()`.
  * [ ] **Verification:** Print a welcome message and parse `BootInfo`, printing framebuffer/memmap details to the serial console.
  * [ ] Enter an infinite `hlt` loop.
  * [ ] **Goal:** Successfully boot and see clean, verifiable output in the QEMU console.

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
