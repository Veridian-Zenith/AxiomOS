# 🏁 Phase One: Bare Metal Bring-Up

**Objective:**
Boot from UEFI firmware into a 64-bit Rust microkernel, establish full control of
memory and execution, and emit verified output via serial and framebuffer.

---

## 1. UEFI Loader (`axiom_loader`)

The loader executes as a UEFI application under OVMF or real firmware.

### Responsibilities

1. **Graphics Output Protocol (GOP)**
   - Locate framebuffer
   - Record base address, resolution, and stride

2. **UEFI Memory Map**
   - Retrieve `EFI_MEMORY_DESCRIPTOR[]`
   - Preserve map key for `ExitBootServices`

3. **System Description Tables**
   - Locate ACPI RSDP
   - Preserve pointer for kernel parsing

4. **Handoff Construction**
   - Populate `AxiomHandoff` in physically contiguous memory

5. **Firmware Exit**
   - Call `ExitBootServices`
   - Disable all firmware ownership

At this point, **the firmware is gone**.

---

## 2. Memory Model & Paging

UEFI identity mappings are discarded.

### Virtual Address Layout

+-------------------------------+
| Kernel (High Half) | 0xFFFFFFFF80000000
+-------------------------------+
| Guard Region |
+-------------------------------+
| User Space |
+-------------------------------+
| Direct Physical Map (DPM) | 0xFFFF800000000000
+-------------------------------+


- **Kernel Base:** `0xFFFFFFFF80000000`
- **Direct Physical Map:** maps all usable RAM for kernel access
- **No implicit identity mapping**

---

## 3. Kernel Entry (`_start`)

The kernel is entered directly from the UEFI loader in 64-bit long mode.

### Initialization Order

1. **CPU State Validation**
   - Verify long mode
   - Verify paging active

2. **Descriptor Tables**
   - Install minimal GDT
   - Install IDT with exception stubs

3. **Serial Output**
   - Initialize UART (COM1)
   - Establish early debug logging

4. **Physical Memory Manager**
   - Parse UEFI memory descriptors
   - Initialize bitmap frame allocator

5. **Idle Loop**
   - Halt safely with interrupts enabled

---

## 4. Hardware Topology Awareness

Phase One performs *enumeration only*.

### Tasks

- Parse ACPI MADT
- Identify logical CPUs
- Classify cores via CPUID leaf `0x1A` (Hybrid Info)

**No SMP bring-up occurs in Phase One.**

---

## 📈 Success Criteria

- [ ] UEFI loader prints status text
- [ ] Framebuffer clears to a solid color
- [ ] Kernel emits serial heartbeat
- [ ] CPU is executing in long mode with AxiomOS-owned paging
- [ ] System halts cleanly without firmware involvement
