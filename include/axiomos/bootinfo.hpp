#ifndef AXIOMOS_BOOTINFO_HPP
#define AXIOMOS_BOOTINFO_HPP

#include <stdint.h>
#include <stddef.h>

namespace axiom {

// Shared memory descriptor format between bootloader and kernel
struct EfiMemoryDescriptor {
    uint32_t mem_type;
    uint32_t pad;
    uint64_t phys_start;
    uint64_t virt_start;
    uint64_t num_pages;
    uint64_t attribute;
} __attribute__((packed));

// The structure passed from the UEFI bootloader to the Kernel
struct BootInfo {
    uint64_t framebuffer_base;
    uint64_t framebuffer_size;
    uint32_t framebuffer_stride;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint32_t framebuffer_format;

    // Memory map
    EfiMemoryDescriptor* efi_memory_map;
    size_t efi_memory_map_size;
    size_t efi_descriptor_size;
    uint32_t efi_descriptor_version;

    uint64_t kernel_entry;
} __attribute__((packed));

} // namespace axiom

#endif // AXIOMOS_BOOTINFO_HPP