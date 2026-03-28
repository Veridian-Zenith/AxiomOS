#include "axiomos/bootinfo.hpp"
#include "axiomos/utils/serial.hpp"
#include "axiomos/x86_64.hpp"
#include "axiomos/mm/allocator.hpp"

namespace {
[[maybe_unused]] void PrintKernelInfo([[maybe_unused]] const axiom::BootInfo* info) {
    using namespace axiom;
    serial::puts("[KERNEL] BootInfo received.\n");
}
}

extern "C" [[noreturn]] void kmain(axiom::BootInfo* info) {
    using namespace axiom;

    // 1. Initialize Serial
    serial::init();
    serial::puts("\n\n");
    serial::puts("========================================================\n");
    serial::puts("  AxiomOS Kernel v0.1.0-alpha (C++26)\n");
    serial::puts("========================================================\n");
    serial::puts("[KERNEL] Early serial initialized.\n");

    if (info) {
        serial::puts("[KERNEL] BootInfo detected.\n");
    } else {
        serial::puts("[KERNEL] WARNING: BootInfo is NULL!\n");
    }

    // 2. Initialize Memory Management
    serial::puts("[KERNEL] Initializing PMM...\n");
    mm::init(info);

    // 3. Hardware initialization
    serial::puts("[KERNEL] Initializing GDT/IDT/APIC...\n");
    arch::x86_64::setupGdt();
    arch::x86_64::setupIdt();
    arch::x86_64::setupApic();

    serial::puts("[KERNEL] System initialized. Entering idle loop.\n");

    // 4. Final safety loop
    while (true) {
        arch::x86_64::halt();
    }
}
