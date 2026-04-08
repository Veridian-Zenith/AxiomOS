#include "memory.h"
#include <Library/UefiBootServicesTableLib.h>

namespace axiom::sentinel {

extern EFI_SYSTEM_TABLE* gST;

void* AllocatePages(UINTN count) {
    EFI_PHYSICAL_ADDRESS address;
    EFI_STATUS status = gST->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, count, &address);
    if (EFI_ERROR(status)) return nullptr;

    // Clear the memory
    for (uint64_t i = 0; i < (count * 4096) / sizeof(uint64_t); ++i) {
        ((uint64_t*)address)[i] = 0;
    }

    return (void*)address;
}

void* AllocatePage() {
    return AllocatePages(1);
}

void MapPage(axiom::PageTable* pml4, uint64_t virtualAddress, uint64_t physicalAddress, uint64_t flags) {
    uint64_t pml4_idx = (virtualAddress >> 39) & 0x1FF;
    uint64_t pdpt_idx = (virtualAddress >> 30) & 0x1FF;
    uint64_t pd_idx   = (virtualAddress >> 21) & 0x1FF;
    uint64_t pt_idx   = (virtualAddress >> 12) & 0x1FF;

    if (!(*pml4)[pml4_idx].Present) {
        (*pml4)[pml4_idx].Address = (uint64_t)AllocatePage() >> 12;
        (*pml4)[pml4_idx].Present = 1;
        (*pml4)[pml4_idx].Writable = 1;
    }

    axiom::PageTable* pdpt = (axiom::PageTable*)((*pml4)[pml4_idx].Address << 12);
    if (!(*pdpt)[pdpt_idx].Present) {
        (*pdpt)[pdpt_idx].Address = (uint64_t)AllocatePage() >> 12;
        (*pdpt)[pdpt_idx].Present = 1;
        (*pdpt)[pdpt_idx].Writable = 1;
    }

    axiom::PageTable* pd = (axiom::PageTable*)((*pdpt)[pdpt_idx].Address << 12);
    if (!(*pd)[pd_idx].Present) {
        (*pd)[pd_idx].Address = (uint64_t)AllocatePage() >> 12;
        (*pd)[pd_idx].Present = 1;
        (*pd)[pd_idx].Writable = 1;
    }

    axiom::PageTable* pt = (axiom::PageTable*)((*pd)[pd_idx].Address << 12);
    (*pt)[pt_idx].Address = physicalAddress >> 12;
    (*pt)[pt_idx].Present = 1;
    (*pt)[pt_idx].Writable = (flags & 0x2) ? 1 : 0;
}

} // namespace axiom::sentinel
