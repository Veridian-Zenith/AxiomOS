#pragma once
#include <cstdint>

namespace axiom {

struct BootInfo {
    uint64_t Signature; // 'AXIOM0'
    uint64_t Revision;

    uint64_t MemoryMapAddress;
    uint64_t MemoryMapSize;
    uint64_t DescriptorSize;

    uint64_t AcpiRsdpAddress;

    struct {
        uint64_t FramebufferBase;
        uint32_t Width;
        uint32_t Height;
        uint32_t PixelsPerScanLine;
        uint32_t Format;
    } GraphicsInfo;

    uint64_t KernelStackTop;
    uint64_t RuntimeServicesTable; // UEFI Runtime Services
};

} // namespace axiom
