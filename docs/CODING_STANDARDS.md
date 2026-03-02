# Code Quality Standards for AxiomOS

## File Headers

Every source file must start with:

```zig
//! Module description: what this file does
//! Architecture: x86_64 (or generic if applicable)
//! Safety: requirements/assumptions
```

## Function Documentation

Every public function must have:

```zig
/// Brief one-line description
/// Parameters:
///   param_name - description
/// Returns: description or void
/// Safety: caller requirements (unsafe blocks, alignment, etc.)
/// Example: (if applicable)
pub fn functionName(param: Type) ReturnType {
    // Implementation
}
```

## Style Guide

- **Naming**: `snake_case` for functions/variables, `PascalCase` for types
- **Line Length**: Max 100 characters
- **Indentation**: 4 spaces (configured in Zig)
- **Comments**: Explain "why", not "what" - the code shows what

## Safety

- All `asm` blocks must be documented with safety requirements
- Use inline functions where performance matters
- Avoid `@ptrCast` without alignment verification
- Document all assumptions about CPU state (e.g., "assumes paging enabled")

## Memory Management

- Use `allocator` pattern throughout
- Avoid zero-sized allocations
- Explicit error handling with try/catch where allocation can fail
- Document allocation lifetime clearly

## Architecture-Specific Code

All CPU instructions in `arch/x86_64.zig`:
- Inline assembly properly escaped
- Register constraints verified
- Clobbered registers documented

## Testing

- Build with `-O ReleaseSmall` for size-critical code
- Use `zig build test` phases (when tests are added)
- Test on real hardware when possible, fallback to QEMU
