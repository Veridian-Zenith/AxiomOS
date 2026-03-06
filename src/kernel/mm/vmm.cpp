// Module description: Virtual Memory Manager (VMM) implementation
// Architecture: x86_64
// Safety: TODO
#include "axiomos/mm/vmm.hpp"
#include "axiomos/utils/serial.hpp"
#include "axiomos/x86_64.hpp"

namespace axiom::mm {

namespace {
    // Pointer to the top-level page table (PML4)
    // We get this from the CR3 register, as it was set up by the bootloader.
    uint64_t* pml4 = nullptr;
}

void initVmm() {
    pml4 = reinterpret_cast<uint64_t*>(arch::x86_64::readCr3());
    serial::printf("[VMM] VMM Initialized. PML4 at: 0x%x\n", (uint64_t)pml4);
    // For now, we just take control of the bootloader's page tables.
    // In the future, we would create a new set of kernel page tables here.
}

// NOTE: The following functions are stubs and need full implementation.
// They require walking the page table hierarchy (PML4 -> PDPT -> PD -> PT).

void mapPage(uint64_t virt_addr, uint64_t phys_addr, uint64_t flags) {
    (void)virt_addr;
    (void)phys_addr;
    (void)flags;
    // TODO: Implement page table walking and mapping
    serial::puts("[VMM] mapPage: Not implemented\n");
}

void unmapPage(uint64_t virt_addr) {
    (void)virt_addr;
    // TODO: Implement page table walking and unmapping
    serial::puts("[VMM] unmapPage: Not implemented\n");
}

uint64_t virtToPhys(uint64_t virt_addr) {
    (void)virt_addr;
    // TODO: Implement page table walking and address translation
    serial::puts("[VMM] virtToPhys: Not implemented\n");
    return 0;
}

} // namespace axiom::mm
