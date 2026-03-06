// Module description: Physical Memory Manager (PMM) using a bitmap allocator.
// Architecture: Generic (relies on bootloader-provided memory map)
// Safety: This module is unsafe. It directly manipulates memory based on
//         the assumption that the bootloader's memory map is accurate. It
//         must be initialized before any other memory-intensive kernel
//         subsystems.
#include "axiomos/mm/pmm.hpp"
#include "axiomos/utils/serial.hpp"

namespace axiom::mm {

namespace {
    // Basic bitmap implementation for Physical Memory Manager
    // 1 bit represents 1 page (4KB)
    // 0 = free, 1 = used
    uint8_t* bitmap = nullptr;
    size_t bitmap_size = 0; // in bytes
    size_t total_pages = 0;
    size_t free_pages = 0;

    // Helper functions for bitmap manipulation
    inline void setBit(size_t bit) {
        bitmap[bit / 8] |= (1 << (bit % 8));
    }

    inline void clearBit(size_t bit) {
        bitmap[bit / 8] &= ~(1 << (bit % 8));
    }

    inline bool testBit(size_t bit) {
        return (bitmap[bit / 8] & (1 << (bit % 8))) != 0;
    }
}

void initPmm(const BootInfo* boot_info) {
    // 1. Calculate total memory to determine bitmap size
    uint64_t max_addr = 0;
    total_pages = 0;

    const uint8_t* mem_map_ptr = reinterpret_cast<const uint8_t*>(boot_info->efi_memory_map);
    size_t num_entries = boot_info->efi_memory_map_size / boot_info->efi_descriptor_size;

    for (size_t i = 0; i < num_entries; ++i) {
        const auto* desc = reinterpret_cast<const EfiMemoryDescriptor*>(
            mem_map_ptr + i * boot_info->efi_descriptor_size
        );

        // Track highest address to know how large the bitmap needs to be
        uint64_t top = desc->phys_start + (desc->num_pages * PAGE_SIZE);
        if (top > max_addr) {
            max_addr = top;
        }
    }

    total_pages = max_addr / PAGE_SIZE;
    bitmap_size = (total_pages + 7) / 8; // Size in bytes, rounded up

    // 2. Find a suitable place to put the bitmap
    // We need a contiguous block of free memory (EfiConventionalMemory = 7)
    // large enough to hold the bitmap.
    for (size_t i = 0; i < num_entries; ++i) {
        const auto* desc = reinterpret_cast<const EfiMemoryDescriptor*>(
            mem_map_ptr + i * boot_info->efi_descriptor_size
        );

        if (desc->mem_type == 7 && (desc->num_pages * PAGE_SIZE) >= bitmap_size) {
            // Ensure the bitmap is placed in the first 4GB (identity mapped by bootloader)
            if (desc->phys_start + bitmap_size <= 0xFFFFFFFF) {
                bitmap = reinterpret_cast<uint8_t*>(desc->phys_start);
                // We found our spot.
                break;
            }
        }
    }

    if (!bitmap) {
        serial::puts("[PMM] Error: Could not find memory for PMM bitmap!\n");
        while (true) {} // Halt
    }

    // 3. Initialize the bitmap: Mark everything as used by default (safe)
    for (size_t i = 0; i < bitmap_size; ++i) {
        bitmap[i] = 0xFF;
    }
    free_pages = 0;

    // 4. Mark actually free regions as 0 (free) in the bitmap
    for (size_t i = 0; i < num_entries; ++i) {
        const auto* desc = reinterpret_cast<const EfiMemoryDescriptor*>(
            mem_map_ptr + i * boot_info->efi_descriptor_size
        );

        // EfiConventionalMemory (7) or EfiBootServicesCode (4) or EfiBootServicesData (5)
        // Note: BootServices memory is safe to use *after* ExitBootServices has been called.
        if (desc->mem_type == 7 || desc->mem_type == 4 || desc->mem_type == 5) {
            size_t start_page = desc->phys_start / PAGE_SIZE;
            for (size_t p = 0; p < desc->num_pages; ++p) {
                clearBit(start_page + p);
                free_pages++;
            }
        }
    }

    // 5. Mark the bitmap's own memory as used so it doesn't allocate itself
    size_t bitmap_start_page = reinterpret_cast<uint64_t>(bitmap) / PAGE_SIZE;
    size_t bitmap_pages = (bitmap_size + PAGE_SIZE - 1) / PAGE_SIZE;
    for (size_t p = 0; p < bitmap_pages; ++p) {
        if (!testBit(bitmap_start_page + p)) {
            setBit(bitmap_start_page + p);
            free_pages--;
        }
    }

    serial::printf("[PMM] Initialized. Total Pages: %d, Free Pages: %d\n", total_pages, free_pages);
}

uint64_t allocPage() {
    return allocPages(1);
}

uint64_t allocPages(size_t count) {
    if (count == 0) return 0;

    // Simple first-fit search
    size_t contiguous = 0;
    size_t start_page = 0;

    for (size_t i = 0; i < total_pages; ++i) {
        if (!testBit(i)) {
            if (contiguous == 0) {
                start_page = i;
            }
            contiguous++;

            if (contiguous == count) {
                // Found enough pages! Mark them as used.
                for (size_t p = 0; p < count; ++p) {
                    setBit(start_page + p);
                }
                free_pages -= count;
                return start_page * PAGE_SIZE;
            }
        } else {
            contiguous = 0; // Reset search
        }
    }

    serial::puts("[PMM] Out of memory!\n");
    return 0; // OOM
}

void freePage(uint64_t phys_addr) {
    freePages(phys_addr, 1);
}

void freePages(uint64_t phys_addr, size_t count) {
    if (phys_addr % PAGE_SIZE != 0) {
        serial::puts("[PMM] Error: freePages called with misaligned address.\n");
        return;
    }

    size_t start_page = phys_addr / PAGE_SIZE;

    // Safety check to avoid out of bounds
    if (start_page + count > total_pages) {
        serial::puts("[PMM] Error: freePages out of bounds.\n");
        return;
    }

    for (size_t p = 0; p < count; ++p) {
        if (testBit(start_page + p)) {
            clearBit(start_page + p);
            free_pages++;
        }
    }
}

uint64_t getTotalMemory() {
    return total_pages * PAGE_SIZE;
}

uint64_t getFreeMemory() {
    return free_pages * PAGE_SIZE;
}

} // namespace axiom::mm
