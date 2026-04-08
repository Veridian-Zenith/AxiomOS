#include "internal.h"
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

extern "C" EFI_STATUS EFIAPI EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    InitializeLib(ImageHandle, SystemTable);

    axiom::sentinel::SentinelConfig config = {
        .EnableFramebuffer = true,
        .EnableAcpi = true
    };

    axiom::sentinel::InitializeFirmware(config);
    axiom::sentinel::LoadKernel();

    return EFI_SUCCESS;
}

namespace axiom::sentinel {

void InitializeFirmware(SentinelConfig config) {
    if (config.EnableFramebuffer) {
        // TODO: Locate GOP, set mode
    }
    if (config.EnableAcpi) {
        // TODO: Locate RSDP
    }
}

void LoadKernel() {
    // TODO: Load kernel ELF binary from ESP
}

void TransitionToKernel(BootInfo* boot_info) {
    // TODO: Construct paging, jump to entry point
}

} // namespace axiom::sentinel
