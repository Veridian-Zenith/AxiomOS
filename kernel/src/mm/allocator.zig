//! Physical Memory Allocator
//! Architecture: x86_64
//! Safety: Bitmap-based allocator for physical page allocation

const std = @import("std");
const serial = @import("../serial.zig");

// ====================================================================
// Constants
// ====================================================================

const PAGE_SIZE = 4096;
const MAX_MEMORY = 1024 * 1024 * 1024; // 1 GB max for now
const MAX_PAGES = MAX_MEMORY / PAGE_SIZE;
const BITMAP_SIZE = (MAX_PAGES + 7) / 8; // bits to bytes

// ====================================================================
// Global State
// ====================================================================

var page_bitmap: [BITMAP_SIZE]u8 align(PAGE_SIZE) = [_]u8{0} ** BITMAP_SIZE;
var total_pages: usize = 0;
var free_pages: usize = 0;

// ====================================================================
// EFI Memory Map Parsing
// ====================================================================

// UEFI Memory Type constants
const EFI_MEMORY_DESCRIPTOR_SIZE = 48; // Standard size

pub const EfiMemoryType = enum(u32) {
    Reserved = 0,
    LoaderCode = 1,
    LoaderData = 2,
    BootServicesCode = 3,
    BootServicesData = 4,
    RuntimeServicesCode = 5,
    RuntimeServicesData = 6,
    Conventional = 7,
    UnusableMemory = 8,
    AcpiReclaimMemory = 9,
    AcpiMemoryNvs = 10,
    MemoryMappedIO = 11,
    MemoryMappedIOPortSpace = 12,
    PalCode = 13,
    PersistentMemory = 14,
);

pub const EfiMemoryDescriptor = packed struct {
    mem_type: u32,
    phys_start: u64,
    virt_start: u64,
    num_pages: u64,
    attribute: u64,
};

/// Parse EFI memory map and initialize allocator
pub fn parseEfiMemoryMap(
    memory_map: [*]u8,
    map_size: usize,
    descriptor_size: usize,
) void {
    var offset: usize = 0;
    var usable_pages: u64 = 0;

    serial.puts("[Memory] Parsing EFI memory map:\n");

    while (offset < map_size) {
        const descriptor: *const EfiMemoryDescriptor = @ptrCast(@alignCast(&memory_map[offset]));

        const mem_type_name = switch (descriptor.mem_type) {
            0 => "Reserved",
            1 => "LoaderCode",
            2 => "LoaderData",
            3 => "BootServicesCode",
            4 => "BootServicesData",
            5 => "RuntimeServicesCode",
            6 => "RuntimeServicesData",
            7 => "Conventional",
            8 => "UnusableMemory",
            9 => "AcpiReclaimMemory",
            10 => "AcpiMemoryNvs",
            11 => "MemoryMappedIO",
            12 => "MemoryMappedIOPortSpace",
            13 => "PalCode",
            14 => "PersistentMemory",
            else => "Unknown",
        };

        // Only usable conventional memory
        if (descriptor.mem_type == 7) { // Conventional memory
            usable_pages += descriptor.num_pages;
            serial.printf("[Memory]  %s: 0x{x:0>12} - 0x{x:0>12} ({} pages)\n",
                mem_type_name, descriptor.phys_start,
                descriptor.phys_start + (descriptor.num_pages * PAGE_SIZE),
                descriptor.num_pages);
        }

        offset += descriptor_size;
    }

    serial.printf("[Memory] Total usable: {} MB\n", (usable_pages * PAGE_SIZE) / (1024 * 1024));
    total_pages = (usable_pages < MAX_PAGES) ? usable_pages : MAX_PAGES;
    free_pages = total_pages;
}

// ====================================================================
// Physical Memory Allocation
// ====================================================================

/// Allocate a single physical page
/// Safety: Returns null if no pages available
pub fn allocatePage() ?u64 {
    var i: usize = 0;
    while (i < BITMAP_SIZE) : (i += 1) {
        if (page_bitmap[i] != 0xFF) {
            // Found a byte with a free bit
            var bit: u3 = 0;
            while (bit < 8) : (bit += 1) {
                if ((page_bitmap[i] & (@as(u8, 1) << bit)) == 0) {
                    // Mark as allocated
                    page_bitmap[i] |= (@as(u8, 1) << bit);
                    free_pages -= 1;

                    const page_num = i * 8 + bit;
                    return @as(u64, @intCast(page_num)) * PAGE_SIZE;
                }
            }
        }
    }
    return null; // No free pages
}

/// Free a physical page
/// Safety: Page must have been allocated
pub fn freePage(page_addr: u64) void {
    const page_num = page_addr / PAGE_SIZE;
    const byte_idx = page_num / 8;
    const bit_idx: u3 = @truncate(page_num % 8);

    if (byte_idx < BITMAP_SIZE) {
        page_bitmap[byte_idx] &= ~(@as(u8, 1) << bit_idx);
        free_pages += 1;
    }
}

/// Get free pages count
pub fn getFreePages() u64 {
    return @as(u64, @intCast(free_pages));
}

// ====================================================================
// Virtual Memory Setup
// ====================================================================

/// Initialize virtual memory paging structures
/// Safety: Must reserve higher-half kernel space (0xffffffff80000000)
pub fn initVirtualMemory() void {
    serial.puts("[Memory] Setting up 4-level page tables...\n");

    // TODO: Set up page directory, tables, and switch to higher-half kernel
    // For now, paging is already enabled by bootloader
}

/// Initialize physical allocator after EFI map parsing
pub fn initPhysicalAllocator() void {
    // Already done during parseEfiMemoryMap
    serial.printf("[Memory] Physical allocator ready: {} pages available\n", free_pages);
}
