# AxiomOS UEFI Loader

The loader is responsible for:

- Executing under UEFI firmware
- Gathering system state
- Preparing the kernel handoff
- Exiting boot services cleanly

It does NOT:
- Manage paging
- Allocate kernel memory
- Contain kernel logic


## 2.2 Loader Design Rules
- Rust + uefi crate
- Runs fully in firmware environment
- No global state
- Produces a single AxiomHandoff structure
