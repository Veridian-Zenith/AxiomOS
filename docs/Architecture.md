# AxiomOS Architecture

This document defines **hard architectural boundaries**.
Violations are bugs, not design differences.

---

## 1. Kernel Responsibilities (Microkernel)

The kernel is responsible for:

- Thread scheduling
- Virtual memory mapping
- Inter-process communication (IPC)
- Capability enforcement
- Context switching

The kernel must remain:
- Small
- Deterministic
- Hardware-agnostic (via HAL only)

---

## 2. What the Kernel MUST NOT Do

The kernel shall NEVER:

- Access hardware devices directly
- Implement filesystems
- Implement networking protocols
- Allocate dynamic user memory
- Contain policy decisions

---

## 3. Hardware Abstraction Layer (HAL)

The HAL provides **mechanical abstractions**, not policy.

HAL responsibilities:
- CPU feature discovery
- Interrupt routing
- MMIO / DMA primitives
- Platform-specific decoding

HAL does NOT:
- Manage devices
- Schedule work
- Own memory

---

## 4. User-Space Services

All system functionality outside the kernel runs as user-space services:

- Device drivers
- Filesystems
- Networking stacks
- Init / system manager

These communicate exclusively via IPC.

---

## 5. IPC as the Backbone

IPC is the only legal communication mechanism between components.

- No shared global state
- No hidden channels
- No implicit trust

If it isn’t IPC, it doesn’t exist.

---

## 6. Capability Model (High-Level)

Authority is represented by explicit capabilities.

- Capabilities are transferable
- Capabilities are revocable
- Possession equals permission

No ambient authority is allowed.
