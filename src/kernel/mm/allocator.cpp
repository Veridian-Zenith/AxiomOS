#include <cstddef>
#include <cstdint>

#include "axiom/mm/allocator.hpp"
#include "axiom/boot/info.hpp"
#include "axiom/drivers/serial.hpp"
#include "axiom/boot/uefi.hpp"


namespace {
    static uint8_t g_bitmap_static[64 * 1024]; // 64KB bitmap
    uint8_t* g_bitmap = g_bitmap_static;
    size_t g_bitmap_size = 64 * 1024;
    size_t g_total_pages = 0;
    size_t g_free_pages = 0;
    size_t g_last_alloc_idx = 0;

    constexpr size_t PAGE_SIZE = 4096;

    // Bitmap manipulation helpers
    inline bool is_set(size_t bit) {
        return g_bitmap[bit / 8] & (1 << (bit % 8));
    }

    inline void set(size_t bit) {
        g_bitmap[bit / 8] |= (1 << (bit % 8));
    }

    inline void clear(size_t bit) {
        g_bitmap[bit / 8] &= ~(1 << (bit % 8));
    }

    void set_range(size_t base, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            set(base + i);
        }
    }

    void clear_range(size_t base, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            clear(base + i);
        }
    }
}

namespace axiom::mm {

void init(const BootInfo* info) {
    serial::puts("[PMM] Initializing.\n");
    uint64_t highest_addr = 0;
    for (size_t i = 0; i < info->mmap_size / info->mmap_desc_size; ++i) {
        auto desc = reinterpret_cast<const axiom::MemoryDescriptor*>(
            reinterpret_cast<const uint8_t*>(info->mmap) + i * info->mmap_desc_size
        );
        uint64_t top = desc->physical_start + (desc->number_of_pages * PAGE_SIZE);
        if (top > highest_addr) {
            highest_addr = top;
        }
    }

    g_total_pages = highest_addr / PAGE_SIZE;

    // Mark all memory as used initially
    for (size_t i = 0; i < g_bitmap_size; ++i) {
        g_bitmap[i] = 0xFF;
    }

    // Mark available memory as free
    for (size_t i = 0; i < info->mmap_size / info->mmap_desc_size; ++i) {
        auto desc = reinterpret_cast<const axiom::MemoryDescriptor*>(
            reinterpret_cast<const uint8_t*>(info->mmap) + i * info->mmap_desc_size
        );

        if (desc->type == uefi::EfiConventionalMemory) {
            size_t base_page = desc->physical_start / PAGE_SIZE;
            clear_range(base_page, desc->number_of_pages);
            g_free_pages += desc->number_of_pages;
        }
    }

    serial::puts("[PMM] Initialized.\n");
}


[[nodiscard]] void* alloc_pages(size_t count) {
    if (count == 0) {
        serial::puts("[PMM-DENIAL] alloc_pages(0)\n");
        asm("cli; hlt");
    }

    size_t free_count = 0;
    size_t start_idx = g_last_alloc_idx;

    for (size_t i = 0; i < g_total_pages; ++i) {
        size_t current_idx = (start_idx + i) % g_total_pages;
        if (!is_set(current_idx)) {
            free_count++;
        } else {
            free_count = 0;
        }

        if (free_count == count) {
            size_t base = current_idx - count + 1;
            set_range(base, count);
            g_free_pages -= count;
            g_last_alloc_idx = base + count;
            return reinterpret_cast<void*>(base * PAGE_SIZE);
        }
    }

    // Retry from beginning if first search failed
    if (g_last_alloc_idx != 0) {
        g_last_alloc_idx = 0;
        return alloc_pages(count);
    }

    return nullptr; // Out of memory
}

void free_pages(void* address, size_t count) {
    serial::puts("[PMM] free_pages\n");
    uint64_t addr = reinterpret_cast<uint64_t>(address);

    if (addr % PAGE_SIZE != 0) {
        serial::puts("[PMM-DENIAL] free_pages with non-page-aligned address.\n");
        asm("cli; hlt");
    }

    size_t base = addr / PAGE_SIZE;

    // Check if within managed range
    if (base + count > g_total_pages) {
        serial::puts("[PMM-DENIAL] Attempted to free memory outside managed range.\n");
        asm("cli; hlt");
    }

    // Check for double-free
    for (size_t i = 0; i < count; ++i) {
        if (!is_set(base + i)) {
            serial::puts("[PMM-DENIAL] double-free detected.\n");
            asm("cli; hlt");
        }
    }

    clear_range(base, count);
    g_free_pages += count;
}


size_t get_free_memory() {
    return g_free_pages * PAGE_SIZE;
}

size_t get_total_memory() {
    return g_total_pages * PAGE_SIZE;
}

} // namespace axiom::mm
