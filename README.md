# AxiomOS: The Silicon Sovereign

A modern, **UEFI-native**, 64-bit operating system for x86_64, built from the ground up in **C++26** using the **LLVM 22.1** toolchain.

> **Note:** AxiomOS is designed to be a portable, hardware-agnostic operating system. While it maintains a direct, high-fidelity relationship with the underlying silicon, it avoids hard-coding hardware-specific dependencies into its core architecture. Details regarding the initial reference hardware and target platform can be found in [**`docs/hardware_info.md`**](./docs/hardware_info.md).

## 👁️ The Vision

AxiomOS is a fundamental rejection of modern software abstraction layers. We don't virtualize; we dominate. By removing the "OS tax" imposed by legacy compatibility and third-party bloat, AxiomOS achieves a direct, high-fidelity relationship with the underlying silicon.

## 🛡️ Core Philosophy: Strictly No Third-Party Bloat

* **Zero-Dependency Kernel:** Every line of the "Registry" core is handwritten or statically verified. No external libraries, no "standard" libc headers that bring hidden baggage.
* **Hardware Authority:** No VT-x, no SVM, and no "guest" support. The kernel assumes absolute Ring 0 authority.
* **Least-Privilege Modularity:** While the kernel owns the hardware, drivers and services are isolated in user-space "Plug-ins" to ensure system-wide resilience.

## 🔐 Security and Signing

AxiomOS maintains a strict digital signature policy to ensure the integrity of the system:

* **Internal Development:** Signing is optional for private, user-side development (e.g., custom drivers or experimental modules).
* **Public Contributions:** All production-ready items contributed to the public AxiomOS repository must include a valid digital signature.

## �️ Architectural Roadmap (v1.0)

The development of AxiomOS is structured into six distinct phases:

0. **Phase 0: The Axiom Standard (The Contract)** - Defining the `.hpp` contract headers and ensuring no global state.
1. **[Phase 1: Sentinel (UEFI Boot-Bridge)](./docs/sentinel_spec.md)** - From firmware to kernel entry.
2. **[Phase 2: Registry (Micro-Monolithic Kernel)](./docs/registry_spec.md)** - Resource sovereignty and ID:0 authority.
3. **[Phase 3: Plug-in I/O (User-Space Drivers)](./docs/plugin_io_spec.md)** - Modular hardware isolation.
4. **[Phase 4: Axiom-VFS (3-Boot Storage)](./docs/axiom_vfs_spec.md)** - High-speed NVMe and XFS integration.
5. **[Phase 5: Fish Shell (Interaction Layer)](./docs/fish_shell_spec.md)** - The "Flow-State" user environment.

## 🏗️ Building AxiomOS

```bash
# Requirements: CMake 3.25+, Clang/LLVM 22.1+, QEMU + OVMF
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build

./tools/run-qemu.sh
```

## ⚖️ License

Licensed under the **Open Software License (OSL) v. 3.0**. See [LICENSE](./LICENSE) for details.
