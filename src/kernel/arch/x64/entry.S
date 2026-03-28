# AxiomOS Kernel Entry Point
# ====================================================================

.section .bss
.align 16
stack_bottom:
    .skip 16384 # 16KiB stack
stack_top:

.section .text
.global _start

_start:
    # 1. Setup Stack
    mov $stack_top, %rsp

    # 2. Ensure Stack is 16-byte aligned (though it should be already)
    and $-16, %rsp

    # 3. Entry argument (BootInfo*) is already in RDI thanks to bootloader's sysv_abi call
    # Just in case, we can push it if we want to follow standard prologue, but not necessary here.

    # 4. Call Kernel Main
    .extern kmain
    call kmain

    # 5. Safety Loop (should never reach here)
_halt:
    cli
    hlt
    jmp _halt
