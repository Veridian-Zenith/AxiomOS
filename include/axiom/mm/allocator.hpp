#pragma once

#include <cstddef>
#include <cstdint>
#include "axiom/boot/info.hpp"

namespace axiom::mm {
    // Resilience Policy for the Physical Memory Manager (PMM)
    //
    // The system prioritizes resilience. For any allocation request, the PMM will:
    // 1. Reasonable Path: Attempt a first-fit search from the last allocation point.
    // 2. Alternative Paths:
    //    a. If the first search fails, restart the search from the beginning of memory.
    //    b. (Future) Attempt to allocate discontiguous pages.
    //    c. (Future) Trigger memory compaction.
    //    d. Fail with a specific error code (e.g., returning a nullptr).
    //
    // Strict Denial (Suspicious Acts):
    // The following actions are considered illogical or malicious and will result in a kernel halt:
    // - alloc_pages(0): Requesting zero pages.
    // - free_pages(address, count) with a non-page-aligned address.
    // - free_pages for memory that is already free (double-free).
    // - free_pages for an address outside the managed range.
    // The kernel will print a verbose [PMM-DENIAL] alert to the serial console before halting.

    /// Initializes the physical memory manager.
    /// This function parses the UEFI memory map and builds a bitmap of available memory.
    /// @param info Pointer to the BootInfo struct from the bootloader.
    void init(const BootInfo* info);

    /// Allocates a contiguous block of physical memory pages.
    /// @param count The number of 4KiB pages to allocate.
    /// @return The physical address of the allocated block, or nullptr if allocation fails.
    [[nodiscard]] void* alloc_pages(size_t count);

    /// Frees a contiguous block of physical memory pages.
    /// @param address The starting physical address of the block to free. Must be page-aligned.
    /// @param count The number of 4KiB pages to free.
    void free_pages(void* address, size_t count);

    /// Gets the amount of free physical memory.
    /// @return The total free memory in bytes.
    size_t get_free_memory();

    /// Gets the total amount of physical memory managed by the PMM.
    /// @return The total memory in bytes.
    size_t get_total_memory();
}
