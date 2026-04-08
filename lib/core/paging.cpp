#include "paging.h"

namespace axiom::paging {

void MapPage(PageTable* pml4, uint64_t virtualAddress, uint64_t physicalAddress, uint64_t flags) {
    // 1. Calculate indices (PML4, PDPT, PD, PT)
    // 2. Traverse tables (allocate if necessary)
    // 3. Set entry
}

void UnmapPage(PageTable* pml4, uint64_t virtualAddress) {
    // ...
}

} // namespace axiom::paging
