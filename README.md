# AxiomOS: Hardware Authority

A modern, **UEFI-only**, 64-bit operating system for x86_64, built from scratch in **C++26** using **LLVM/Clang 22.1**.

AxiomOS is a rejection of modern software bloat. We don't do "virtual systems," hypervisors, or generic abstraction layers that throttle performance. This is a high-fidelity, native-only implementation designed to own the silicon.

---

## 🛠 Project Philosophy

* **Zero-Layer Execution:** No VT-x, no SVM, and no "guest" support. The kernel assumes absolute Ring 0 authority.
* **Logical Identity Mapping:** We use the MMU for protection bits, but favor Identity Mapping to keep the logic lean—what you see in the code is what the hardware executes.
* **Hardware-Specific Optimization:** Primary target is the **HP ProBook 450 G9** (Intel i3-1215U, Alder Lake). We optimize for the **2 Performance cores** and **4 Efficiency cores** natively.
* **Modern Toolchain:** 100% LLVM-native. We leverage `std::expected`, `consteval`, and advanced pack indexing to keep the binary tight and fast.

**Refer to [hardware_info.md](./docs/hardware_info.md) for the detailed base system specifications.**

---

## 🚀 Current Status: Phase 1 (COMPLETED)

* **UEFI Bootloader:** Hand-written EFI application that handles ELF loading and memory map hand-off.
* **Higher-Half Kernel:** Successfully mapped and executing at `0xFFFFFFFF80000000`.
* **Early Logging:** UART 16550 serial driver for COM1 debug output.
* **Memory PMM:** Bitmap-based physical memory allocator is operational.

---

## 🗺 Roadmap

Refer to [ROADMAP.md](./docs/ROADMAP.md) for detailed phase breakdown and future plans.

---

## 📦 Building & Testing

```fish
# Requirements: CMake 3.25+, Clang/LLVM 22.1+, QEMU + OVMF
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

./tools/run-qemu.sh
```

---

## ⚖️ License

Licensed under the **Open Software License (OSL) v. 3.0**. See [LICENSE](./LICENSE) for details.
