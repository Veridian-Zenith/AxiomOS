#include "pmm.h"

namespace axiom::pmm {

void Initialize(void* memoryMap, uint64_t mapSize, uint64_t descriptorSize) {
    // 1. Parse UEFI Memory Map
    // 2. Build bitmap/list of free pages
}

void* AllocatePage() {
    // 1. Find free page in bitmap
    // 2. Mark as used
    // 3. Return address
    return nullptr;
}

void FreePage(void* address) {
    // 1. Mark as free
}

} // namespace axiom::pmm
