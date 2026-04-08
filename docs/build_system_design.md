# AxiomOS Build System Design

This document outlines the design for the AxiomOS build system using CMake and Ninja.

## Core Requirements

- **Language:** C++26
- **Toolchain:** LLVM 22.1 (freestanding)
- **Target:** x86_64-unknown-uefi (for bootloader) and x86_64-unknown-none (for kernel/userspace)
- **Modular Structure:** Component-based builds per module (`src/kernel/*`, `src/user/*`).

## Toolchain Configuration

The project will use a dedicated `cmake/toolchain.cmake` file to define cross-compilation parameters.

### Compiler Flags

The flags need careful partitioning:

#### Kernel Flags (Freestanding)

- `-ffreestanding`
- `-fno-exceptions`
- `-fno-rtti`
- `-std=c++26`
- `-Wall -Wextra -Werror`

#### User-space Flags (Supportive environment)

These flags (provided by the user) can be adapted for non-kernel components (e.g., shell, tools) where runtime libraries are available:

- `-march=native -O3 -pipe -fno-plt -rtlib=compiler-rt -unwindlib=libunwind`
- `-Wno-unused-variable -Wno-unused-parameter -Wno-unused-function -Wno-unused-but-set-variable`
- `-Wno-missing-field-initializers -Wno-sign-compare -Wno-unused-result`
- `-g -fstandalone-debug`
- `-fno-omit-frame-pointer -fno-optimize-sibling-calls`

*Note: Sanitizers (`-fsanitize`) are not applicable to the kernel core without explicit kernel-level sanitizer support.*

## Optimization

- **ThinLTO:** Enabled by default (`-flto=thin`) across all modules for cross-module optimization and faster link times.

## CMake Structure

1. **Root `CMakeLists.txt`:** Defines project scope, enables languages, includes toolchain, and adds subdirectories.
2. **Component `CMakeLists.txt`:** Each component (e.g., `src/kernel/registry/CMakeLists.txt`) will define its own library or executable target.
3. **Library Targets:** Modules will be built as static libraries or objects to be linked into the final kernel image.

## Workflow

1. Configure build: `cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain.cmake -G Ninja`
2. Build: `ninja -C build`

This modular approach ensures that each component can be tested and built independently while maintaining the integrity of the overall OS.
