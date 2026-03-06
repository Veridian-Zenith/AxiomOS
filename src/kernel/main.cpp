// Module description: Main entry point for the AxiomOS kernel.
// Architecture: x86_64
// Safety: This module assumes it is entered from the bootloader in a
//         64-bit long mode environment with paging enabled.
#include "axiomos/bootinfo.hpp"
#include "axiomos/utils/serial.hpp"
#include "axiomos/x86_64.hpp"
#include "axiomos/mm/pmm.hpp"
#include "axiomos/mm/vmm.hpp"
#include "axiomos/arch/x86_64/cpuid.hpp"

extern "C" {
/// @brief The main entry point of the AxiomOS kernel.
/// @param boot_info A pointer to the BootInfo structure passed from the bootloader.
///                  This structure contains vital information about the memory map,
///                  framebuffer, and other system details.
/// @note This function is marked with C linkage to be callable from assembly.
/// @note Safety: This function should never return. It enters an infinite halt loop
///               upon completion of initialization.
void kmain(axiom::BootInfo* boot_info) {
    // 1. Initialize serial port for debugging
    axiom::serial::init();

    axiom::serial::puts("=====================================\n");
    axiom::serial::puts("[AxiomOS C++ Kernel] Initializing...\n");
    axiom::serial::puts("=====================================\n");

    // Print some boot info
    axiom::serial::printf("Framebuffer Base: 0x%x\n", boot_info->framebuffer_base);
    axiom::serial::printf("Resolution: %dx%d\n", boot_info->framebuffer_width, boot_info->framebuffer_height);

    // Detect CPU
    axiom::arch::x86_64::printCpuInfo();

    // 2. Setup Architecture (GDT, IDT, PIC)
    axiom::serial::puts("[Kernel] Setting up GDT...\n");
    axiom::arch::x86_64::setupGdt();

    axiom::serial::puts("[Kernel] Setting up IDT...\n");
    axiom::arch::x86_64::setupIdt();

    axiom::serial::puts("[Kernel Remapping Legacy PIC...\n");
    axiom::arch::x86_64::initPic();

    // 3. Initialize Memory Manager
    axiom::serial::puts("[Kernel] Initializing Physical Memory Manager...\n");
    axiom::mm::initPmm(boot_info);

    axiom::serial::puts("[Kernel] Initializing Virtual Memory Manager...\n");
    axiom::mm::initVmm();

    axiom::serial::printf("[Kernel] Total Memory: %llu KB\n", axiom::mm::getTotalMemory() / 1024);
    axiom::serial::printf("[Kernel] Free Memory: %llu KB\n", axiom::mm::getFreeMemory() / 1024);
    axiom::serial::puts("\n");
    axiom::serial::puts("=====================================\n");
    axiom::serial::puts("[AxiomOS System Idle.\n");
    axiom::serial::puts("=====================================\n");

    // Halt the CPU continuously
    while (true) {
        axiom::arch::x86_64::halt();
    }
}
}

