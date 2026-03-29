#include "axiom/arch/x64/idt.hpp"
#include "axiom/arch/x64/cpu.hpp"
#include "axiom/drivers/serial.hpp"
#include "axiom/arch/x64/private/gdt.hpp"

namespace axiom::arch::x64 {

static IdtEntry idt[256] __attribute__((aligned(16)));
static IdtPointer idtp;

// ISR stubs from assembly
extern "C" void isr_stub_0();
extern "C" void isr_stub_1();
extern "C" void isr_stub_2();
extern "C" void isr_stub_3();
extern "C" void isr_stub_4();
extern "C" void isr_stub_5();
extern "C" void isr_stub_6();
extern "C" void isr_stub_7();
extern "C" void isr_stub_8();
extern "C" void isr_stub_9();
extern "C" void isr_stub_10();
extern "C" void isr_stub_11();
extern "C" void isr_stub_12();
extern "C" void isr_stub_13();
extern "C" void isr_stub_14();
extern "C" void isr_stub_15();
extern "C" void isr_stub_16();
extern "C" void isr_stub_17();
extern "C" void isr_stub_18();
extern "C" void isr_stub_19();
extern "C" void isr_stub_20();
extern "C" void isr_stub_21();
extern "C" void isr_stub_22();
extern "C" void isr_stub_23();
extern "C" void isr_stub_24();
extern "C" void isr_stub_25();
extern "C" void isr_stub_26();
extern "C" void isr_stub_27();
extern "C" void isr_stub_28();
extern "C" void isr_stub_29();
extern "C" void isr_stub_30();
extern "C" void isr_stub_31();

void setIdtGate(uint8_t vector, void (*handler)(), uint8_t ist, uint8_t type) {
    uint64_t addr = (uint64_t)handler;
    idt[vector].isr_low = addr & 0xFFFF;
    idt[vector].selector = KERNEL_CODE_SEL;
    idt[vector].ist = ist;
    idt[vector].attributes = type;
    idt[vector].isr_mid = (addr >> 16) & 0xFFFF;
    idt[vector].isr_high = (addr >> 32) & 0xFFFFFFFF;
    idt[vector].reserved = 0;
}

void initIdt() {
    idtp.limit = (uint16_t)sizeof(idt) - 1;
    idtp.base = (uint64_t)&idt;

    // Populate the first 32 entries (Exceptions)
    setIdtGate(0, isr_stub_0);
    setIdtGate(1, isr_stub_1);
    setIdtGate(2, isr_stub_2);
    setIdtGate(3, isr_stub_3);
    setIdtGate(4, isr_stub_4);
    setIdtGate(5, isr_stub_5);
    setIdtGate(6, isr_stub_6);
    setIdtGate(7, isr_stub_7);
    setIdtGate(8, isr_stub_8);
    setIdtGate(9, isr_stub_9);
    setIdtGate(10, isr_stub_10);
    setIdtGate(11, isr_stub_11);
    setIdtGate(12, isr_stub_12);
    setIdtGate(13, isr_stub_13);
    setIdtGate(14, isr_stub_14);
    setIdtGate(15, isr_stub_15);
    setIdtGate(16, isr_stub_16);
    setIdtGate(17, isr_stub_17);
    setIdtGate(18, isr_stub_18);
    setIdtGate(19, isr_stub_19);
    setIdtGate(20, isr_stub_20);
    setIdtGate(21, isr_stub_21);
    setIdtGate(22, isr_stub_22);
    setIdtGate(23, isr_stub_23);
    setIdtGate(24, isr_stub_24);
    setIdtGate(25, isr_stub_25);
    setIdtGate(26, isr_stub_26);
    setIdtGate(27, isr_stub_27);
    setIdtGate(28, isr_stub_28);
    setIdtGate(29, isr_stub_29);
    setIdtGate(30, isr_stub_30);
    setIdtGate(31, isr_stub_31);

    // Load IDT
    __asm__ volatile("lidt %0" : : "m"(idtp));
}

struct CpuState {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t vector, error_code;
    uint64_t rip, cs, rflags, rsp, ss;
} __attribute__((packed));

extern "C" void exception_handler(CpuState* state) {
    using namespace axiom;
    serial::puts("\n\n!!!! EXCEPTION DETECTED !!!!\n");
    serial::puts("[KERNEL] Vector: ");
    serial::puthex(state->vector);
    serial::puts("\n[KERNEL] Error Code: ");
    serial::puthex(state->error_code);
    serial::puts("\n[KERNEL] RIP: ");
    serial::puthex(state->rip);
    serial::puts("\n\nHALTING SYSTEM.");
    while (true) arch::x64::halt();
}

} // namespace axiom::arch::x64
