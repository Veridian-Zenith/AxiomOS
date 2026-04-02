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
    # Early DEBUG: print "AXIOM" to COM1 (0x3F8)
    mov $0x3F8, %dx
    mov $'A', %al
    out %al, %dx
    mov $'X', %al
    out %al, %dx
    mov $'I', %al
    out %al, %dx
    mov $'O', %al
    out %al, %dx
    mov $'M', %al
    out %al, %dx
    mov $'\n', %al
    out %al, %dx

    # 1. Setup Stack
    mov $stack_top, %rsp

    # 2. Ensure Stack is 16-byte aligned (though it should be already)
    and $-16, %rsp

    # 3. Call Kernel Main
    .extern kmain
    call kmain

    # 4. Safety Loop
_halt:
    cli
    hlt
    jmp _halt
