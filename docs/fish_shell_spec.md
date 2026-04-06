# Fish Shell Specification (Interaction Layer)

## 1. Overview

The Fish shell is the primary interaction layer for AxiomOS. It is a high-performance, ergonomic command-line environment and user interface designed for developers and power users. It leverages the underlying system's hybrid architecture and graphics capabilities to provide a "Flow-State" experience.

## 2. Telemetry Integration

The Fish shell is not just a command runner; it is a real-time system monitor.

- **Real-time Metrics:** CPU usage (specifically P-core vs. E-core utilization), system memory pressure, NVMe throughput, and thermal data are integrated directly into the shell's status bar or as an overlay.
- **Service Status:** Monitors the health and status of active Plug-in I/O drivers and core Registry services.
- **Hardware Feedback:** Visualizes Intel Thread Director decisions and thermal throttling events as they occur.

## 3. Signed-Execution Enforcement

To maintain system integrity (as defined in Phase 2), the Fish shell enforces strict execution policies.

- **Signature Verification:** Before attempting to execute any binary, the shell requests a signature check from the Registry (ID:0 Authority).
- **Execution Denial:** The shell will refuse to spawn a process for any unsigned or incorrectly signed executable, providing a detailed error message and logging the event.
- **Trust Level Indicators:** Binaries are visually tagged in the shell's autocomplete (e.g., green for signed system core, yellow for signed third-party, red for unsigned).

## 4. Flow-State UI Concepts

The Fish shell is designed for maximum productivity with minimal distractions.

- **Sub-Millisecond Latency:** The shell's input-to-render pipeline is highly optimized, ensuring that user interaction feels instantaneous.
- **Context-Aware Autocomplete:** Suggestions are prioritized based on the current system state, such as pinned files in Axiom-VFS or active services in the Registry.
- **Direct Terminal Rendering:** Uses the Intel UHD Graphics (64 EUs) via a native DRM/KMS-like interface for high-resolution, hardware-accelerated terminal rendering. This bypasses legacy VGA/BIOS modes entirely.
- **Integration with VFS:** Direct shell commands for managing pinned files and monitoring NVMe health, including visibility into the **3-Boot Purge** schedule.

## 5. System V ABI Userspace

Fish shell provides a standard environment for modern applications.

- **Standard ABI:** Full support for applications compiled using the x86_64 System V ABI and C++26.
- **Dynamic Loading:** Supports dynamic linking through a custom AxiomOS loader that interfaces with the Registry for service resolution.
- **Process Management:** Robust job control (fg, bg, jobs) and high-fidelity signal handling.

## 6. Customization & Ergonomics

- **Syntax Highlighting:** Real-time, asynchronous syntax highlighting that does not impede typing performance.
- **Theming:** A modular theme system that supports high-contrast and low-latency rendering modes.
- **Plugin System:** A sandboxed, signed plugin system for extending shell functionality without compromising system stability.
