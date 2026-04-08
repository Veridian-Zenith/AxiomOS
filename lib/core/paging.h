#pragma once
#include <axiom/memory.h>

namespace axiom::paging {

void MapPage(PageTable* pml4, uint64_t virtualAddress, uint64_t physicalAddress, uint64_t flags);
void UnmapPage(PageTable* pml4, uint64_t virtualAddress);

} // namespace axiom::paging
