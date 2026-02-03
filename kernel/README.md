# AxiomOS Kernel

The kernel is a Rust `no_std` microkernel.

Phase One responsibilities:
- Accept control from loader
- Initialize minimal CPU state
- Output debug text via serial
- Halt safely

No IPC, no scheduling, no user mode **yet**.
