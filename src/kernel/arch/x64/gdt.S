# AxiomOS GDT Loader
# ====================================================================

.global gdt_load
.global tss_load

# void gdt_load(GdtPointer* ptr, uint16_t code, uint16_t data)
# RDI = ptr, RSI = code, RDX = data
gdt_load:
    lgdt (%rdi)

    # Reload segment registers
    mov %dx, %ds
    mov %dx, %es
    mov %dx, %fs
    mov %dx, %gs
    mov %dx, %ss

    # Push far return parameters
    # The return address is ALREADY on the stack.
    # We need to do a far return to set CS.
    
    # 1. Save return address
    pop %rax
    
    # 2. Push CS (RSI)
    push %rsi
    
    # 3. Push return address (RAX)
    push %rax
    
    # 4. Far return
    lretq

# void tss_load(uint16_t sel)
# RDI = sel
tss_load:
    ltr %di
    ret
