# Plug-in I/O Specification (User-Space Drivers)

## 1. Overview

Plug-in I/O is the AxiomOS driver framework. To enhance system stability and security, drivers run in isolated user-space contexts (Ring 3) while retaining high-performance access to hardware through a high-speed IPC and the kernel's Registry.

### 1.1 Initial Targets

The Phase 3 execution focuses on three critical hardware paths:

- **UART (16550):** Legacy debug and telemetry output, transitioned from polling (Phase 2) to interrupt-driven (Phase 3).
- **Storage Controller:** Primary storage path, leveraging modern interrupt mechanisms (e.g., MSI-X) and direct submission queues.
- **Graphics Subsystem:** Framebuffer control and early hardware acceleration (GOP transition to native DRM).

## 2. Driver Development Strategy

To accelerate hardware support while maintaining the "Silicon Sovereign" long-term vision, AxiomOS employs a three-tiered evolution for drivers:

1. **Port & Adapt:** Initial support for complex hardware (e.g., Intel SOF audio, Xe graphics) is achieved by porting and adapting existing proven logic from mature stacks like Linux. These "Ported Plugins" run in the standard Plug-in I/O user-space environment.
2. **Firmware Utilization:** Where legally and technically feasible, existing binary firmware is used to bypass the most complex aspects of hardware initialization (e.g., DSP topology loading, hardware-specific microcode).
3. **Native Transition:** As the system matures, these ported implementations are iteratively refactored into "Native Plug-in I/O" modules, optimized specifically for AxiomOS's SM-IPC and zero-copy paradigms.

## 3. Shared-Memory IPC (SM-IPC)

SM-IPC is the primary data path for driver communication, bypassing traditional kernel system calls for data transit.

- **Ring Buffers:** Lockless, circular buffers reside in shared memory between the driver and its clients (or the kernel).
- **Control Plane:** Small, fixed-size messages (syscall-based) for setting up buffers and signaling status.
- **Data Plane:** Large data transfers occur entirely in shared memory with zero-copy semantics.
- **Virtual Interrupts:** Kernel-to-driver signaling mechanism using specialized eventfd-like constructs to notify drivers of hardware IRQs.

## 3. PCIe Enumeration (Platform Controller Hub)

The kernel discovers PCIe devices and delegates management to Plug-in I/O.

- **MCFG Retrieval:** The Registry retrieves the PCI Express configuration space base address from the ACPI MCFG table.
- **Bus Scanning:** Recursive enumeration of buses, devices, and functions.
- **Capability Discovery:**
  - **MSI / MSI-X:** Configuring message-signaled interrupts to be delivered directly to the driver process.
  - **BAR Mapping:** Memory-Mapped I/O (MMIO) ranges are mapped into the driver's address space.
- **IOMMU (Intel VT-d):** If available, the kernel configures VT-d to isolate DMA operations to the driver's specific memory region, preventing cross-process memory corruption.

## 4. Hot-Swap Mechanisms

AxiomOS treats all external devices (and some internal PCH components) as potentially hot-swappable.

- **Event Detection:** Listens for ACPI "Device Check" or "Eject Request" events.
- **Dynamic Binding:**
  - **Plugin:** When a device is inserted, the Registry identifies the appropriate driver, spawns its process, and maps the BARs.
  - **Unplug:** When a device is removed, the Registry immediately unmaps MMIO and signals the driver to terminate or enter a "disconnected" state.
- **State Serialization:** Drivers are encouraged to periodically serialize their hardware state to ephemeral storage for faster recovery during unplanned disconnects.

## 5. USB 3.2 xHCI Driver (Target Implementation)

The primary high-speed peripheral interface for the target platform.

- **Controller Context:** The xHCI host controller's operational registers are mapped into the USB driver's user-space.
- **Ring Management:** Command, Event, and Transfer rings are maintained in driver-owned memory that the controller accesses via DMA.
- **Isochronous/Bulk/Interrupt:** Optimized scheduling for different USB transfer types using the Hybrid-Affinity scheduler to ensure low-latency audio/video processing.

## 6. Security & Isolation

- **Resource Limits:** Drivers are strictly limited in their CPU and memory usage to prevent a malfunctioning driver from starving the system.
- **Signed Plugins:** All Plug-in I/O drivers must be digitally signed and verified by the Registry (ID:0 Authority) before execution.
