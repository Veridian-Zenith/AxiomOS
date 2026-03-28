#include "axiom/mm/allocator.hpp"
#include "axiom/boot/info.hpp"

namespace axiom::mm {

void init(const BootInfo* info) {
    // TODO: Implement physical memory management using bitmap allocator
    (void)info;
}

} // namespace axiom::mm
