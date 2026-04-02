# AxiomOS Kernel Entry Point
# ====================================================================

.section .bss
.align 16
stack_bottom:
    .skip 16384 # 16KiB stack
stack_top:

.section .text._start
.global _start

_start:
    # Save BootInfo pointer (passed in rdi)
    mov %rdi, %r12
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

    # Debug: Print "1" (Before stack)
    mov $0x3F8, %dx
    mov $'1', %al
    out %al, %dx
    mov $'\n', %al
    out %al, %dx

    # 1. Setup Stack
    mov $stack_top, %rsp

    # 2. Setup GDT
    mov $gdt_ptr, %rdi
    mov $0x08, %rsi
    mov $0x10, %rdx
    call gdt_load

    # 3. Far jump to higher-half
    lea 1f(%rip), %rax
    movabs $0xFFFFFFFF80000000, %rbx
    or %rbx, %rax
    push $0x08
    push %rax
    lretq

1:
    # Now in higher-half

    # 4. Ensure Stack is 16-byte aligned (though it should be already)
    and $-16, %rsp

    # Debug: Print "3" (Before kmain)
    mov $0x3F8, %dx
    mov $'3', %al
    out %al, %dx
    mov $'\n', %al
    out %al, %dx

    # 5. Call Kernel Main
    mov %r12, %rdi
    .extern kmain
    call kmain

    # 6. Safety Loop
_halt:
    cli
    hlt
    jmp _halt

.section .rodata
gdt_start:
    .quad 0x0000000000000000 # Null entry
    .quad 0x00209A0000000000 # Kernel Code (64-bit)
    .quad 0x0000920000000000 # Kernel Data
gdt_end:

gdt_ptr:
    .word gdt_end - gdt_start - 1
    .quad gdt_start
