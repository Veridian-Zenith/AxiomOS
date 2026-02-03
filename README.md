# 🌌 AxiomOS
> A capability-oriented, zero-waste microkernel for modern x86_64 systems.

AxiomOS is a from-scratch operating system built around **strict modularity** and
**explicit ownership**. The kernel is intentionally minimal: it exists only to
schedule execution, map memory, and securely route inter-process communication (IPC).

Everything else runs as isolated user-space services.

---

## 🧠 Design Principles

- **Anti-Monolithic:** No drivers, filesystems, or protocol stacks in the kernel.
- **Capability-Based Security:** Authority is explicit, transferable, and minimal.
- **Zero-Copy by Default:** Data moves via remapping, not memcpy.
- **Zero-Waste Design:** If code does not affect correctness or performance, it does not exist.

---

## 🏗️ Architecture Overview

AxiomOS uses a polyglot architecture to balance safety, control, and performance:

- **UEFI Loader (Rust):**
  - Executes under OVMF or real firmware
  - Collects system state (memory map, framebuffer, ACPI)
  - Transfers ownership to the kernel

- **Microkernel Core (Rust, `no_std`):**
  - Thread scheduling
  - Virtual memory management
  - IPC routing
  - Capability enforcement

- **Hardware Abstraction Layer (C++):**
  - Zero-cost abstractions over CPU, interrupts, MMIO, and DMA
  - Architecture-aware, platform-neutral interfaces

- **User-Space System Servers (Rust / C++):**
  - Device drivers
  - Filesystems
  - Networking stacks

---

## 🛠️ Developer Environment (Fedora 43)

| Tool        | Version | Purpose                  c        |
| ----------- | ------- | -------------------------------- |
| Rustc       | 1.93+   | Kernel & UEFI loader             |
| Clang / LLD | 21.1+   | HAL & cross-language linking     |
| GCC         | 15.2+   | Low-level C glue                 |
| NASM        | 2.16+   | Context switch & interrupt stubs |
| QEMU        | 10.1+   | x86_64 emulation (q35 + OVMF)    |

---

## 🚀 Status

AxiomOS is currently in **Phase One: Bare Metal Bring-Up**.

See `docs/Phase_ONE.md` for details.
