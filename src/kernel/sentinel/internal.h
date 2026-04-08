#pragma once
#include <axiom/boot_info.h>
#include <cstdint>

namespace axiom::sentinel {

// Internal Sentinel configuration
struct SentinelConfig {
    bool EnableFramebuffer;
    bool EnableAcpi;
};

// Internal boot sequence
void InitializeFirmware(SentinelConfig config, BootInfo* boot_info);
struct KernelLoadResult {
    uint64_t EntryPoint;
    axiom::PageTable* Pml4;
};
KernelLoadResult LoadKernel(EFI_HANDLE ImageHandle);
void TransitionToKernel(BootInfo* boot_info, uint64_t entry_point, axiom::PageTable* pml4, uint64_t stack_top);

} // namespace axiom::sentinel
