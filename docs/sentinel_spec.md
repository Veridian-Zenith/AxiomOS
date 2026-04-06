# Sentinel Specification (UEFI Boot-Bridge)

## 1. Overview

The Sentinel is the AxiomOS bootloader, responsible for transitioning the system from a UEFI environment to the AxiomOS kernel (The Registry). It ensures system integrity, discovers critical hardware via ACPI, and establishes the initial execution context.

## 2. UEFI Runtime & Boot Services

Sentinel utilizes UEFI services to interact with the firmware before exiting boot services:

- **Memory Map:** Obtained via `GetMemoryMap()`. This map is translated into the AxiomOS physical memory descriptor format.
- **GOP (Graphics Output Protocol):** Locates the `EFI_GRAPHICS_OUTPUT_PROTOCOL` to initialize the Graphics framebuffer.
- **File I/O:** Loads the kernel ELF64 binary from the EFI System Partition (ESP).

## 3. ACPI Discovery

To support the target platform's architecture, Sentinel must perform deep ACPI parsing:

- **RSDP/XSDT:** Locate the Root System Description Pointer and traverse the Extended System Descriptor Table (XSDT) using 64-bit physical addresses.
- **MADT (Multiple APIC Description Table):**
  - Enumerate all Local APICs to identify the cores properly.
  - Locate I/O APICs for interrupt routing.
  - Identify NMI sources and Local APIC Address overrides.
- **FADT (Fixed ACPI Description Table):** Retrieve hardware-reduced ACPI flags and power management register blocks.

## 4. GOP Framebuffer Setup

- **Mode Selection:** Query available modes and select the highest resolution supported by the eDP/HDMI/VGA interface.
- **Handover Info:**
  - `BaseAddress`: Physical start of the framebuffer.
  - `BufferSize`: Total size in bytes.
  - `Width / Height`: Horizontal and vertical resolution.
  - `PixelsPerScanLine`: For correct stride calculation.
  - `PixelFormat`: Usually BGRX or RGBX 8-bit.

## 5. ELF64 Kernel Loader

- **Validation:** Verify ELF magic, architecture (x86_64), and type (ET_EXEC or ET_DYN).
- **Segment Mapping:**
  - Iterate through Program Headers.
  - Map `PT_LOAD` segments into the higher-half virtual address space (`0xFFFFFFFF80000000`).
  - Apply page protections: `.text` (R-X), `.rodata` (R--), `.data/.bss` (RW-).

## 6. Handover Protocol (The System Map)

Sentinel passes a pointer to a `BootInfo` structure (System Map) to the kernel entry point in the primary argument register (e.g., `%rdi` on x86_64). The protocol ensures a clean transition with all firmware state finalized.

```cpp
struct BootInfo {
    uint64_t Signature; // 'AXIOM0'
    uint64_t Revision;

    uint64_t MemoryMapAddress;
    uint64_t MemoryMapSize;
    uint64_t DescriptorSize;

    uint64_t AcpiRsdpAddress;

    struct {
        uint64_t FramebufferBase;
        uint32_t Width;
        uint32_t Height;
        uint32_t PixelsPerScanLine;
        uint32_t Format;
    } GraphicsInfo;

    uint64_t KernelStackTop;
    uint64_t RuntimeServicesTable; // UEFI Runtime Services
};
```

### 6.1 Handover Refinements

- **Firmware Silence:** All UEFI timers and background processes are halted before jump.
- **Identity Unmap:** The kernel is responsible for unmapping the identity-mapped bootstrap pages once the higher-half transition is complete.
- **CPU State:** All processors (except BSP) are placed in a known `WAIT_FOR_SIPI` state or equivalent.

## 7. Higher-Half Transition

- **Paging:** Construct a bootstrap PML4 table.
- **Identity Map:** Map the first 2MiB or 1GiB to allow the transition.
- **Kernel Map:** Map the kernel starting at `-2GiB` (`0xFFFFFFFF80000000`).
- **Jump:** Load the new CR3, transition to the kernel stack, and `jmp` to the kernel entry point.
