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
void InitializeFirmware(SentinelConfig config);
void LoadKernel();
void TransitionToKernel(BootInfo* boot_info);

} // namespace axiom::sentinel
