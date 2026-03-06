#ifndef AXIOMOS_MM_PMM_HPP
#define AXIOMOS_MM_PMM_HPP

#include <stdint.h>
#include <stddef.h>
#include "axiomos/bootinfo.hpp"

namespace axiom::mm {

constexpr size_t PAGE_SIZE = 4096;

/// @brief Initialize the physical memory manager using the EFI memory map
void initPmm(const BootInfo* boot_info);

/// @brief Allocate a single physical page (4KB)
/// @return Physical address of the allocated page, or 0 if out of memory
uint64_t allocPage();

/// @brief Allocate multiple contiguous physical pages
/// @param count Number of pages to allocate
/// @return Physical address of the first allocated page, or 0 if out of memory
uint64_t allocPages(size_t count);

/// @brief Free a previously allocated physical page
/// @param phys_addr Physical address of the page to free
void freePage(uint64_t phys_addr);

/// @brief Free multiple contiguous physical pages
/// @param phys_addr Physical address of the first page to free
/// @param count Number of pages to free
void freePages(uint64_t phys_addr, size_t count);

/// @brief Get total system memory in bytes
uint64_t getTotalMemory();

/// @brief Get available (free) system memory in bytes
uint64_t getFreeMemory();

} // namespace axiom::mm

#endif // AXIOMOS_MM_PMM_HPP