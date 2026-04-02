#include "axiom/boot/info.hpp"
#include "axiom/drivers/serial.hpp"
#include "axiom/arch/x64/cpu.hpp"
#include "axiom/arch/x64/paging.hpp"
#include "axiom/mm/allocator.hpp"
#include "axiom/tests/pmm.hpp"

namespace {
[[maybe_unused]] void PrintKernelInfo([[maybe_unused]] const axiom::BootInfo* info) {
    using namespace axiom;
    serial::puts("[KERNEL] BootInfo received.\n");
}
}

extern "C" [[noreturn]] void kmain([[maybe_unused]] axiom::BootInfo* info) {
    using namespace axiom;

    // 1. Initialize Serial
    serial::init();
    serial::puts("[KERNEL] kmain started.\n");

    // 2. Initialize Memory Management
    serial::puts("[KERNEL] Initializing PMM...\n");
    mm::init(info);
    serial::puts("[KERNEL] PMM initialized.\n");
    serial::puts("\n\n");
    serial::puts("========================================================\n");
    serial::puts("  AxiomOS Kernel v0.1.0-alpha (C++26)\n");
    serial::puts("========================================================\n");
    serial::puts("[KERNEL] Early serial initialized.\n");

    tests::test_pmm();

    serial::puts("[KERNEL] Initializing Recursive Paging...\n");
    arch::x64::init_recursive_paging();

    // 3. Hardware initialization
    serial::puts("[KERNEL] Initializing GDT/IDT/APIC...\n");
    arch::x64::setupGdt();
    arch::x64::setupIdt();
    arch::x64::setupApic();

    serial::puts("[KERNEL] System initialized. Entering idle loop.\n");

    // 4. Final safety loop
    while (true) {
        arch::x64::halt();
    }
}
