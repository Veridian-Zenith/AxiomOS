# AxiomOS Directory Structure Proposal

This proposal outlines a modular directory structure for AxiomOS to support component-based architecture, clear kernel/user space separation, and maintainability.

## Proposed Structure

```text
/
├── include/                # Public headers
│   └── axiom/              # Namespace for AxiomOS
├── lib/                    # Shared libraries/Core components
│   ├── core/               # Low-level primitives (formerly axiom_core)
│   └── utils/              # General utilities
├── src/                    # Source code
│   ├── kernel/             # Kernel-space components
│   │   ├── sentinel/
│   │   ├── registry/
│   │   ├── vfs/
│   │   ├── io/             # Plug-in I/O implementation
│   │   └── drivers/
│   └── user/               # User-space components
│       └── shell/
├── tests/                  # Unified testing
└── tools/                  # Build/utility tools
```

## Rationale

- **`include/axiom/`**: Ensures clear, encapsulated public API definition and proper inclusion paths.
- **`lib/`**: Separates core libraries (e.g., `lib/core` for fundamental primitives) from domain-specific logic.
- **`src/kernel/`**: Logical grouping for kernel components, enabling clear separation (Sentinel, Registry, VFS, I/O, Drivers). This modular approach allows for better isolation and maintainability of kernel subsystems.
- **`src/user/`**: Isolates user-space applications, such as the shell, from the kernel.
- **`tests/`**: Centralizes testing, separating source code from validation logic to streamline build and test processes.
- **`src/kernel/io/`**: Added to explicitly house the Plug-in I/O subsystem requirements.
