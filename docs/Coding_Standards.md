# AxiomOS Coding Standards

These rules exist to preserve clarity and correctness.

---

## Rust

- Kernel code MUST use `#![no_std]`
- `unsafe` requires:
  - A comment explaining **why**
  - A description of the invariant being upheld
- No heap allocation in early boot

---

## C++

- No RTTI
- No exceptions
- No STL containers in HAL
- Prefer plain structs and interfaces

---

## Assembly (NASM)

- Assembly is for mechanics only:
  - Context switching
  - Interrupt stubs
- No logic
- No policy
- No control flow decisions beyond necessity

---

## General

- One module = one responsibility
- Files over ~300 lines must justify existence
- Explicit over clever
