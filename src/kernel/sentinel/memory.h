#pragma once
#include <Uefi.h>
#include <axiom/memory.h>

namespace axiom::sentinel {

void* AllocatePage();
void MapPage(axiom::PageTable* pml4, uint64_t virtualAddress, uint64_t physicalAddress, uint64_t flags);

} // namespace axiom::sentinel
