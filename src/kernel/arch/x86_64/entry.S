// Module description: Kernel early entry point.
// Architecture: x86_64
// Safety: Must be entered from a UEFI bootloader in 64-bit long mode.
//         Paging must be enabled and the kernel mapped to the higher half.
.section .text.entry
.global entry
.type entry, @function

// The bootloader jumps here with:
// - 64-bit paging enabled
// - CPU in long mode
// - rcx = pointer to BootInfo structure (UEFI x64 ABI)
entry:
    // Ensure the CPU is in a clean state (clear direction flag)
    cld

    // The Kernel (System V ABI) expects the first argument in rdi.
    // The Bootloader (UEFI ABI) passed it in rcx.
    mov %rcx, %rdi

    // Jump into C++ main
    call kmain

    // If kmain returns, halt infinitely
1:
    hlt
    jmp 1b
