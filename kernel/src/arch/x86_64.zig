//! x86_64 Architecture Support
//! Architecture: x86_64
//! Safety: CPU instructions and privilege operations

const std = @import("std");

// ====================================================================
// CPU Instructions
// ====================================================================

/// Halt the CPU (infinite loop)
pub fn halt() noreturn {
    while (true) {
        asm volatile ("hlt");
    }
}

/// Enable interrupts globally
pub inline fn enableInterrupts() void {
    asm volatile ("sti");
}

/// Disable interrupts globally
pub inline fn disableInterrupts() void {
    asm volatile ("cli");
}

/// Read CPU timestamp counter
pub inline fn readTsc() u64 {
    return asm volatile ("rdtsc"
        : [ret] "=A" (-> u64),
    );
}

/// Read CR0 register
pub inline fn readCr0() u64 {
    return asm volatile ("movq %%cr0, %[ret]"
        : [ret] "=r" (-> u64),
    );
}

/// Write CR0 register
pub inline fn writeCr0(value: u64) void {
    asm volatile ("movq %[val], %%cr0"
        :
        : [val] "r" (value),
    );
}

/// Read CR2 register (page fault address)
pub inline fn readCr2() u64 {
    return asm volatile ("movq %%cr2, %[ret]"
        : [ret] "=r" (-> u64),
    );
}

/// Read CR3 register (page directory base)
pub inline fn readCr3() u64 {
    return asm volatile ("movq %%cr3, %[ret]"
        : [ret] "=r" (-> u64),
    );
}

/// Write CR3 register (page directory base)
pub inline fn writeCr3(value: u64) void {
    asm volatile ("movq %[val], %%cr3"
        :
        : [val] "r" (value),
    );
}

/// Read RFLAGS register
pub inline fn readRflags() u64 {
    return asm volatile ("pushfq; popq %[ret]"
        : [ret] "=r" (-> u64),
    );
}

/// Write RFLAGS register
pub inline fn writeRflags(value: u64) void {
    asm volatile ("pushq %[val]; popfq"
        :
        : [val] "r" (value),
    );
}

/// Load GDT
pub inline fn loadGdt(gdt_ptr: *const GdtDescriptor) void {
    asm volatile ("lgdt %[gdt]"
        :
        : [gdt] "m" (gdt_ptr.*),
    );
}

/// Load IDT
pub inline fn loadIdt(idt_ptr: *const IdtDescriptor) void {
    asm volatile ("lidt %[idt]"
        :
        : [idt] "m" (idt_ptr.*),
    );
}

/// Load Task Register
pub inline fn loadTr(selector: u16) void {
    asm volatile ("ltr %[sel]"
        :
        : [sel] "r" (selector),
    );
}

// ====================================================================
// GDT (Global Descriptor Table)
// ====================================================================

pub const GdtDescriptor = packed struct {
    limit: u16,
    base: u64,
};

pub const GdtEntry = packed struct {
    limit_low: u16,
    base_low: u16,
    base_mid: u8,
    access: u8,
    limit_high_and_flags: u8,
    base_high: u8,
};

pub const GDT_KERNEL_CODE = 0x08;
pub const GDT_KERNEL_DATA = 0x10;
pub const GDT_USER_CODE = 0x18;
pub const GDT_USER_DATA = 0x20;

var gdt: [10]GdtEntry align(16) = undefined;
var gdt_descriptor: GdtDescriptor = undefined;

/// Set up Global Descriptor Table
/// Safety: CPU must support long mode
pub fn setupGdt() void {
    // Null descriptor
    gdt[0] = GdtEntry{ .limit_low = 0, .base_low = 0, .base_mid = 0, .access = 0, .limit_high_and_flags = 0, .base_high = 0 };

    // Kernel code (64-bit)
    gdt[1] = GdtEntry{
        .limit_low = 0xFFFF,
        .base_low = 0,
        .base_mid = 0,
        .access = 0x9A, // Present, DPL=0, Code, Read
        .limit_high_and_flags = 0x20, // 64-bit, granularity
        .base_high = 0,
    };

    // Kernel data
    gdt[2] = GdtEntry{
        .limit_low = 0xFFFF,
        .base_low = 0,
        .base_mid = 0,
        .access = 0x92, // Present, DPL=0, Data, Write
        .limit_high_and_flags = 0x00,
        .base_high = 0,
    };

    // User code
    gdt[3] = GdtEntry{
        .limit_low = 0xFFFF,
        .base_low = 0,
        .base_mid = 0,
        .access = 0xFA, // Present, DPL=3, Code, Read
        .limit_high_and_flags = 0x20,
        .base_high = 0,
    };

    // User data
    gdt[4] = GdtEntry{
        .limit_low = 0xFFFF,
        .base_low = 0,
        .base_mid = 0,
        .access = 0xF2, // Present, DPL=3, Data, Write
        .limit_high_and_flags = 0x00,
        .base_high = 0,
    };

    gdt_descriptor.limit = @sizeOf(@TypeOf(gdt)) - 1;
    gdt_descriptor.base = @intFromPtr(&gdt);

    loadGdt(&gdt_descriptor);

    // Reload code segment using far jump (handled by calling code)
    // Reload data segments
    asm volatile (
        "movw %[ds], %%ax\n"
        "movw %%ax, %%ds\n"
        "movw %%ax, %%es\n"
        "movw %%ax, %%fs\n"
        "movw %%ax, %%gs\n"
        "movw %%ax, %%ss\n"
        :
        : [ds] "i" (GDT_KERNEL_DATA),
    );
}

// ====================================================================
// IDT (Interrupt Descriptor Table)
// ====================================================================

pub const IdtDescriptor = packed struct {
    limit: u16,
    base: u64,
};

pub const IdtEntry = packed struct {
    offset_low: u16,
    selector: u16,
    ist: u8,
    type_attr: u8,
    offset_mid: u16,
    offset_high: u32,
    reserved: u32,
};

var idt: [256]IdtEntry align(4096) = undefined;
var idt_descriptor: IdtDescriptor = undefined;

/// Set up Interrupt Descriptor Table
pub fn setupIdt() void {
    var i: u16 = 0;
    while (i < 256) : (i += 1) {
        idt[i] = IdtEntry{
            .offset_low = 0,
            .selector = 0,
            .ist = 0,
            .type_attr = 0,
            .offset_mid = 0,
            .offset_high = 0,
            .reserved = 0,
        };
    }

    idt_descriptor.limit = @sizeOf(@TypeOf(idt)) - 1;
    idt_descriptor.base = @intFromPtr(&idt);

    loadIdt(&idt_descriptor);
}

// ====================================================================
// PIC/APIC Initialization
// ====================================================================

const PIC1_CMD = 0x20;
const PIC1_DATA = 0x21;
const PIC2_CMD = 0xA0;
const PIC2_DATA = 0xA1;

const ICW1_ICW4 = 0x01;
const ICW1_SINGLE = 0x02;
const ICW1_INTERVAL4 = 0x04;
const ICW1_LEVEL = 0x08;
const ICW1_INIT = 0x10;

const ICW4_8086 = 0x01;
const ICW4_AUTO = 0x02;
const ICW4_BUF_SLAVE = 0x08;
const ICW4_BUF_MASTER = 0x0C;
const ICW4_SFNM = 0x10;

/// Initialize legacy 8259 Programmable Interrupt Controller
/// Safety: Must be called during early boot
pub fn initPic() void {
    // TODO: Detect if system is APIC-capable and use that instead
    // For now, initialize legacy PIC

    // ICW1: send ICW1 to both PICs
    portOut(PIC1_CMD, ICW1_INIT | ICW1_ICW4);
    portOut(PIC2_CMD, ICW1_INIT | ICW1_ICW4);

    // ICW2: set interrupt vector offsets
    portOut(PIC1_DATA, 0x20); // IRQ0-7 -> INT 0x20-0x27
    portOut(PIC2_DATA, 0x28); // IRQ8-15 -> INT 0x28-0x2F

    // ICW3: set master/slave relationship
    portOut(PIC1_DATA, 0x04); // IR2 is slave
    portOut(PIC2_DATA, 0x02); // IR2 connected to master

    // ICW4: set 8086 mode
    portOut(PIC1_DATA, ICW4_8086);
    portOut(PIC2_DATA, ICW4_8086);

    // Mask all interrupts for now
    portOut(PIC1_DATA, 0xFF);
    portOut(PIC2_DATA, 0xFF);
}

/// I/O port out
inline fn portOut(port: u16, value: u8) void {
    asm volatile ("outb %[val], %[port]"
        :
        : [val] "a" (value),
          [port] "N" (port),
    );
}

/// I/O port in
inline fn portIn(port: u16) u8 {
    return asm volatile ("inb %[port], %[val]"
        : [val] "=a" (-> u8),
        : [port] "N" (port),
    );
}

// ====================================================================
// Timer Initialization
// ====================================================================

/// Initialize PIT (Programmable Interval Timer)
/// Sets up 1ms timer interrupt
pub fn initTimer() void {
    // TODO: Implement PIT initialization for periodic timer
}
