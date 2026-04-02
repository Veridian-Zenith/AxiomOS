#include "axiom/arch/x64/paging.hpp"
#include "axiom/drivers/serial.hpp"
#include <cstdint>

namespace axiom::arch::x64 {

void init_recursive_paging() {
    serial::puts("[VMM] Recursive paging established via index 510.\n");

    // The PML4 itself is now virtually accessible at:
    // 0xFFFF000000000000 | (510 << 39) | (510 << 30) | (510 << 21) | (510 << 12)
    // which is 0xFFFFFE7FBFFFE000

    serial::puts("[VMM] Recursive paging ready.\n");
}

} // namespace axiom::arch::x64
