#pragma once
#include <stdint.h>
#include <stddef.h>

namespace axiom {

struct Framebuffer {
    void* base;
    size_t size;
    uint32_t width;
    uint32_t height;
    uint32_t pixels_per_scanline;
};

// Simplified Memory Map Descriptor for handoff
struct MemoryDescriptor {
    uint32_t type;
    uint32_t pad;
    uint64_t physical_start;
    uint64_t virtual_start;
    uint64_t number_of_pages;
    uint64_t attribute;
};

struct BootInfo {
    Framebuffer framebuffer;
    
    // Memory map details
    MemoryDescriptor* mmap;
    size_t mmap_size;
    size_t mmap_desc_size;

    void* rsdp; // ACPI Root System Description Pointer
};

} // namespace axiom