// Module description: Virtual Memory Manager (VMM)
// Architecture: x86_64
// Safety: TODO
#ifndef AXIOMOS_MM_VMM_HPP
#define AXIOMOS_MM_VMM_HPP

#include <stdint.h>
#include <stddef.h>

namespace axiom::mm {

    /// @brief Initializes the Virtual Memory Manager
    /// @note This function takes control of the page tables set up by the bootloader.
    void initVmm();

    /// @brief Maps a virtual page to a physical page
    /// @param virt_addr The virtual address to map
    /// @param phys_addr The physical address to map to
    /// @param flags Page flags (e.g., writable, user, etc.)
    void mapPage(uint64_t virt_addr, uint64_t phys_addr, uint64_t flags);

    /// @brief Unmaps a virtual page
    /// @param virt_addr The virtual address to unmap
    void unmapPage(uint64_t virt_addr);

    /// @brief Get the physical address corresponding to a virtual address
    /// @param virt_addr The virtual address to translate
    /// @return The corresponding physical address, or 0 if not mapped
    uint64_t virtToPhys(uint64_t virt_addr);

} // namespace axiom::mm

#endif // AXIOMOS_MM_VMM_HPP
