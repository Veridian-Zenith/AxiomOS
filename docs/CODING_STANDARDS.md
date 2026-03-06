# Code Quality Standards for AxiomOS

## File Headers

Every source file must start with:

```cpp
// Module description: what this file does
// Architecture: x86_64 (or generic if applicable)
// Safety: requirements/assumptions
```

## Function Documentation

We use Doxygen-style comments for functions:

```cpp
/// @brief Brief one-line description
/// @param param_name description
/// @return description or void
/// @note Safety: caller requirements (unsafe operations, alignment, etc.)
ReturnType functionName(Type param_name) {
    // Implementation
}
```

## Style Guide

- **Naming**: `camelCase` for variables and functions, `PascalCase` for classes/structs and namespaces.
- **Line Length**: Max 120 characters
- **Indentation**: 4 spaces
- **Comments**: Explain "why", not "what". The code should explain "what".
- **Headers**: `#pragma once` is fine, but standard `#ifndef` include guards are preferred.
- **Namespaces**: Use nested `namespace axiom::submodule {}` to avoid pollution. Never use `using namespace xyz` globally.

## C++ Restrictions

- **Exceptions**: Disabled (`-fno-exceptions`). Return `std::expected` (if polyfilled) or a custom Result type.
- **RTTI**: Disabled (`-fno-rtti`). Do not use `dynamic_cast` or `typeid`.
- **Standard Library**: This is a freestanding environment. You only have access to `<cstdint>`, `<cstddef>`, `<type_traits>`, and similar header-only basics. `new`/`delete` and `malloc`/`free` are strictly controlled through custom memory allocators.
- **Global Objects**: Objects with complex constructors/destructors at global scope require a `__cxa_atexit` runtime and initialization loop. Avoid global complex objects; use POD types or lazy initialization.

## Safety

- All `__asm__ volatile` blocks must be wrapped in clean, well-documented helper inline functions.
- Limit `reinterpret_cast` usage. Ensure alignment requirements are explicitly handled and checked.

## Architecture-Specific Code

All CPU instructions in `arch/x86_64/`:
- `__attribute__((packed))` is fully supported and optimized by Clang/LLVM. Ensure packed structs are aligned safely if accessed via pointers.
