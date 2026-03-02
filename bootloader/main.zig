//! UEFI Bootloader for AxiomOS
//! Architecture: x86_64
//! Safety: Must preserve registers, stack, and exit boot services properly

const std = @import("std");
const uefi = std.os.uefi;
const assert = std.debug.assert;

// ====================================================================
// UEFI Protocol GUIDs (required for GOP, memory, etc.)
// ====================================================================

const GRAPHICS_OUTPUT_PROTOCOL_GUID = uefi.Guid{
    .time_low = 0x9042a9de,
    .time_mid = 0x23dc,
    .time_high_and_version = 0x4a38,
    .clock_seq_high_and_reserved = 0x96,
    .clock_seq_low = 0xfb,
    .node = [_]u8{ 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a },
};

// ====================================================================
// Bootloader State & Globals
// ====================================================================

var boot_info: BootInfo = undefined;

pub const BootInfo = struct {
    framebuffer_base: u64,
    framebuffer_size: u64,
    framebuffer_stride: u32,
    framebuffer_width: u32,
    framebuffer_height: u32,
    framebuffer_format: u32,

    efi_memory_map: []uefi.tables.EfiMemoryDescriptor,
    efi_memory_map_size: usize,
    efi_descriptor_size: usize,
    efi_descriptor_version: u32,

    kernel_entry: u64,
};

// ====================================================================
// Program Entry Point (EFI Entry)
// ====================================================================

/// Main UEFI bootloader entry point
/// Parameters:
///   image_handle - Current image handle
///   system_table - EFI system table pointer
/// Returns: EFI status code
/// Safety: System in UEFI boot services mode, must exit boot services before kernel handoff
pub fn efi_main(
    image_handle: uefi.Handle,
    system_table: *uefi.tables.SystemTable,
) uefi.Status {
    // Set up console output first for debugging
    _ = system_table.con_out.?.clearScreen();
    _ = system_table.con_out.?.setMode(0);

    const con_out = system_table.con_out orelse {
        return uefi.Status.InvalidParameter;
    };

    _ = con_out.outputString(u"[AxiomOS] Bootloader starting...\r\n");

    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    // Step 1: Get memory map
    _ = con_out.outputString(u"[AxiomOS] Fetching EFI memory map...\r\n");
    const memory_map = getEfiMemoryMap(allocator, system_table) catch |err| {
        logError(u"Failed to get memory map", err);
        return uefi.Status.OutOfResources;
    };
    defer allocator.free(memory_map);

    boot_info.efi_memory_map_size = memory_map.len;
    boot_info.efi_descriptor_size = @sizeOf(uefi.tables.EfiMemoryDescriptor);
    boot_info.efi_descriptor_version = 1;

    // Step 2: Initialize GOP framebuffer
    _ = con_out.outputString(u"[AxiomOS] Initializing graphics output (GOP)...\r\n");
    initializeGop(system_table) catch |err| {
        logError(u"GOP initialization failed, continuing with serial output", err);
        boot_info.framebuffer_base = 0;
        boot_info.framebuffer_size = 0;
    };

    // Step 3: Load kernel from disk (placeholder - load from bootloader binary for now)
    _ = con_out.outputString(u"[AxiomOS] Kernel image prepared (linked)...\r\n");
    boot_info.kernel_entry = 0xffffffff80000000; // Higher-half kernel entry

    // Step 4: Exit boot services and transfer control to kernel
    _ = con_out.outputString(u"[AxiomOS] Exiting boot services...\r\n");

    var map_key: usize = 0;
    const exit_status = system_table.boot_services.?.exitBootServices(image_handle, &map_key);

    if (exit_status != uefi.Status.Success) {
        _ = con_out.outputString(u"[AxiomOS] Failed to exit boot services\r\n");
        return exit_status;
    }

    // At this point, only UEFI runtime services are available
    // Transfer control to kernel
    const kernel_entry: *const fn (info: *BootInfo) noreturn = @ptrFromInt(boot_info.kernel_entry);
    kernel_entry(&boot_info);

    return uefi.Status.Success;
}

// ====================================================================
// Helper Functions
// ====================================================================

/// Fetch complete EFI memory map
fn getEfiMemoryMap(
    allocator: std.mem.Allocator,
    system_table: *uefi.tables.SystemTable,
) ![]uefi.tables.EfiMemoryDescriptor {
    const boot_services = system_table.boot_services orelse return error.NoBootServices;

    var map_size: usize = 0;
    var map_key: usize = 0;
    var descriptor_size: usize = 0;
    var descriptor_version: u32 = 0;

    // First call to get size
    _ = boot_services.getMemoryMap(
        &map_size,
        null,
        &map_key,
        &descriptor_size,
        &descriptor_version,
    );

    // Allocate with some buffer for safety
    map_size += descriptor_size * 10;
    const buffer = try allocator.alloc(u8, map_size);
    var descriptors: [*]uefi.tables.EfiMemoryDescriptor = @ptrCast(buffer.ptr);

    const status = boot_services.getMemoryMap(
        &map_size,
        descriptors,
        &map_key,
        &descriptor_size,
        &descriptor_version,
    );

    if (status != uefi.Status.Success) {
        allocator.free(buffer);
        return error.MemoryMapFailed;
    }

    const count = map_size / descriptor_size;
    return descriptors[0..count];
}

/// Initialize Graphics Output Protocol (GOP) framebuffer
fn initializeGop(system_table: *uefi.tables.SystemTable) !void {
    const boot_services = system_table.boot_services orelse return error.NoBootServices;

    // Locate GOP protocol
    var gop_handles: [*]uefi.Handle = undefined;
    var gop_handle_count: usize = 0;

    const status = boot_services.locateHandleBuffer(
        uefi.tables.LocateSearchType.ByProtocol,
        &GRAPHICS_OUTPUT_PROTOCOL_GUID,
        null,
        &gop_handle_count,
        &gop_handles,
    );

    if (status != uefi.Status.Success or gop_handle_count == 0) {
        return error.GopNotFound;
    }

    // Get the first GOP device
    var gop: ?*uefi.protocols.GraphicsOutputProtocol = null;
    const gop_status = boot_services.handleProtocol(
        gop_handles[0],
        &GRAPHICS_OUTPUT_PROTOCOL_GUID,
        @ptrCast(&gop),
    );

    if (gop_status != uefi.Status.Success) {
        return error.GopHandleProtocolFailed;
    }

    const gop_proto = gop orelse return error.GopNull;

    // Set mode to preferred native resolution (mode 0 is usually best)
    const set_status = gop_proto.setMode(0);
    if (set_status != uefi.Status.Success) {
        return error.GopSetModeFailed;
    }

    // Get mode information
    const mode_info = gop_proto.mode.*.info;
    boot_info.framebuffer_base = gop_proto.mode.*.frameBufferBase;
    boot_info.framebuffer_size = gop_proto.mode.*.frameBufferSize;
    boot_info.framebuffer_width = mode_info.horizontalResolution;
    boot_info.framebuffer_height = mode_info.verticalResolution;
    boot_info.framebuffer_stride = mode_info.pixelsPerScanLine;
    boot_info.framebuffer_format = @intFromEnum(mode_info.pixelFormat);
}

/// Helper to log errors to console
fn logError(comptime msg: []const u8, err: anytype) void {
    _ = msg;
    _ = err;
    // Would implement proper error logging here
}
