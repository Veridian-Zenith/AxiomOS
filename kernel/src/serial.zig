//! Serial UART Output (8250/16550 compatible)
//! Architecture: x86_64
//! Safety: Direct I/O port access, must only be called in kernel mode

const std = @import("std");

// ====================================================================
// UART Configuration (COM1: 0x3F8)
// ====================================================================

const UART_BASE_PORT = 0x3F8;
const UART_DATA_OFFSET = 0;
const UART_IER_OFFSET = 1; // Interrupt Enable Register
const UART_IIR_OFFSET = 2; // Interrupt Identification Register
const UART_LCR_OFFSET = 3; // Line Control Register
const UART_MCR_OFFSET = 4; // Modem Control Register
const UART_LSR_OFFSET = 5; // Line Status Register
const UART_MSR_OFFSET = 6; // Modem Status Register

const UART_LCR_8N1 = 0x03; // 8 bits, no parity, 1 stop bit
const UART_LCR_DLAB = 0x80; // Divisor Latch Access Bit

const UART_IER_NONE = 0x00;
const UART_MCR_DTR = 0x01;
const UART_MCR_RTS = 0x02;
const UART_MCR_OUT2 = 0x08;

const UART_LSR_THR_EMPTY = 0x20;

const UART_BAUD_DIVISOR = 1; // 115200 baud (for 1.8432 MHz crystal)

// ====================================================================
// I/O Port Access (x86_64 inline assembly)
// ====================================================================

/// Write byte to I/O port
inline fn portOut(comptime port: u16, value: u8) void {
    asm volatile ("outb %[val], %[port]"
        :
        : [val] "a" (value),
          [port] "N" (port),
    );
}

/// Read byte from I/O port
inline fn portIn(comptime port: u16) u8 {
    return asm volatile ("inb %[port], %[val]"
        : [val] "=a" (-> u8),
        : [port] "N" (port),
    );
}

// ====================================================================
// Serial Initialization
// ====================================================================

/// Initialize UART for serial output
/// Safety: Must be called in kernel mode, only once
pub fn init() void {
    // Disable interrupts
    portOut(UART_BASE_PORT + UART_IER_OFFSET, UART_IER_NONE);

    // Set DLAB to configure baud rate
    portOut(UART_BASE_PORT + UART_LCR_OFFSET, UART_LCR_DLAB);

    // Set divisor for 115200 baud
    portOut(UART_BASE_PORT + UART_DATA_OFFSET, @as(u8, @truncate(UART_BAUD_DIVISOR)));
    portOut(UART_BASE_PORT + UART_IER_OFFSET, @as(u8, @truncate(UART_BAUD_DIVISOR >> 8)));

    // Clear DLAB, set 8N1
    portOut(UART_BASE_PORT + UART_LCR_OFFSET, UART_LCR_8N1);

    // Enable FIFO
    portOut(UART_BASE_PORT + UART_IIR_OFFSET, 0xC7);

    // Set RTS+DTR
    portOut(UART_BASE_PORT + UART_MCR_OFFSET, UART_MCR_DTR | UART_MCR_RTS | UART_MCR_OUT2);
}

// ====================================================================
// Serial Output Functions
// ====================================================================

/// Wait until transmit buffer is empty
inline fn waitForTransmit() void {
    while ((portIn(UART_BASE_PORT + UART_LSR_OFFSET) & UART_LSR_THR_EMPTY) == 0) {
        // Spin
    }
}

/// Write single character to serial port
pub fn putchar(ch: u8) void {
    waitForTransmit();
    portOut(UART_BASE_PORT + UART_DATA_OFFSET, ch);

    // Handle carriage return: convert LF to CRLF
    if (ch == '\n') {
        waitForTransmit();
        portOut(UART_BASE_PORT + UART_DATA_OFFSET, '\r');
    }
}

/// Write null-terminated string
pub fn puts(str: [*:0]const u8) void {
    var i: usize = 0;
    while (str[i] != 0) {
        putchar(str[i]);
        i += 1;
    }
}

/// Printf-style formatted output
pub fn printf(comptime format: []const u8, args: anytype) void {
    var buf: [256]u8 = undefined;
    const len = std.fmt.bufPrint(&buf, format, args) catch |err| {
        _ = err;
        return;
    };
    for (len) |ch| {
        putchar(ch);
    }
}

/// Printf with newline
pub fn printfln(comptime format: []const u8, args: anytype) void {
    printf(format ++ "\n", args);
}
