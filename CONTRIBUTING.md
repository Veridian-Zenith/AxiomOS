# Contributing to AxiomOS

AxiomOS is a high-fidelity project requiring precision and strict adherence to architectural standards. We welcome contributions that align with our "Silicon Sovereign" philosophy.

## 🛠 Technical Requirements

To maintain our performance and security guarantees, all contributions must adhere to the following stack:

* **Language:** C++26 (strictly freestanding for kernel components).
* **Compiler:** LLVM / Clang 22.1+.
* **Standards:** Zero-cost abstractions only. No exceptions, no RTTI, no third-party libraries unless explicitly approved.

## 🏗 The Axiom Standard

All module development must strictly adhere to **The Axiom Standard** (defined in Phase 0):

1. **Contract Headers (.hpp):** Interfaces must be defined in pure header files that specify the "contract" between modules. These headers must be zero-dependency and contains no implementation details that would leak internal state.
2. **No Global State:** Modules are strictly forbidden from maintaining global or static state. All context must be explicitly passed through interface methods.
3. **Standalone Verifiability:** Each module must be buildable and testable in isolation.

## 🛡 "Least-Privilege" Kernel Design

AxiomOS follows a strict "Least-Privilege" model for hardware access:

1. **Registry Core:** Only handles memory mapping, scheduling, and interrupt routing.
2. **Plug-in I/O:** All hardware drivers (USB, NVMe, Graphics) must run in isolated user-space contexts.
3. **Isolation:** Use the `ServiceRegistry` and `SM-IPC` for communication between modules. Never bypass isolation layers unless optimizing a "hot path" approved by the core maintainers.

## 🔑 ID:0 Digital Signature Workflow

Security is not an afterthought in AxiomOS. All code executed within the system must be signed.

* **Digital Signatures (ID:0):** Every kernel module and system service must be signed using the Ed25519 algorithm.
* **Verification:** The Registry core (ID:0 Authority) verifies the signature of every binary before it is mapped into the address space.
* **Contributor Workflow:**
  * Pull requests must include a signature manifest for any new binaries or modules.
  * Development builds can use a local "Debug Trust Root," but production-ready code must pass the central signature audit.

## 🚀 How to Contribute

### 1. Identify a Phase

Review the `TODO.md` and `ROADMAP.md` files. We are currently executing in order from Phase 1 through Phase 5.

### 2. Discussion First

Before starting major work, open a **GitHub Discussion** in the "Architecture" category. This ensures your design aligns with the Alder Lake optimization goals.

### 3. Submission Standards

* **Clang-Format:** Use the provided `.clang-format` (LLVM style).
* **Documentation:** All new functions must include Doxygen-style comments explaining hardware-level side effects.
* **Testing:** Kernel changes must include a corresponding test case in `src/kernel/tests/`.

---

Thank you for helping us reclaim the silicon! 🖥️🛡️
