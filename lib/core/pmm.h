#pragma once
#include <cstdint>
#include <cstddef>

namespace axiom::pmm {

void Initialize(void* memoryMap, uint64_t mapSize, uint64_t descriptorSize);
void* AllocatePage();
void FreePage(void* address);

} // namespace axiom::pmm
