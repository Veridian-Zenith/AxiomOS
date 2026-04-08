#include "internal.h"
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/GraphicsOutput.h>
#include <Guid/Acpi.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>
#include <axiom/elf.h>

extern EFI_SYSTEM_TABLE* gST;

extern "C" EFI_STATUS EFIAPI EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    InitializeLib(ImageHandle, SystemTable);

    axiom::sentinel::SentinelConfig config = {
        .EnableFramebuffer = true,
        .EnableAcpi = true
    };

    axiom::sentinel::InitializeFirmware(config);
    axiom::sentinel::LoadKernel(ImageHandle);

    return EFI_SUCCESS;
}

namespace axiom::sentinel {

EFI_FILE_PROTOCOL* OpenKernelFile(EFI_HANDLE ImageHandle) {
    EFI_LOADED_IMAGE_PROTOCOL* loadedImage = nullptr;
    gST->BootServices->HandleProtocol(ImageHandle, &gLoadedImageProtocolGuid, (void**)&loadedImage);

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fs = nullptr;
    gST->BootServices->HandleProtocol(loadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void**)&fs);

    EFI_FILE_PROTOCOL* root = nullptr;
    fs->OpenVolume(fs, &root);

    EFI_FILE_PROTOCOL* kernelFile = nullptr;
    root->Open(root, &kernelFile, (CHAR16*)L"axiom_kernel.elf", EFI_FILE_MODE_READ, 0);

    return kernelFile;
}

void InitializeFirmware(SentinelConfig config) {
    if (config.EnableFramebuffer) {
        EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
        EFI_GRAPHICS_OUTPUT_PROTOCOL* gop = nullptr;

        EFI_STATUS status = gST->BootServices->LocateProtocol(&gopGuid, nullptr, (void**)&gop);
        if (EFI_ERROR(status)) {
            // Log error
        } else {
            // Successfully located GOP
        }
    }
    if (config.EnableAcpi) {
        EFI_GUID acpi2Guid = ACPI_20_TABLE_GUID;
        void* rsdp = nullptr;

        for (UINTN i = 0; i < gST->NumberOfTableEntries; ++i) {
            if (CompareGuid(&gST->ConfigurationTable[i].VendorGuid, &acpi2Guid)) {
                rsdp = gST->ConfigurationTable[i].VendorTable;
                break;
            }
        }

        if (rsdp) {
            // Successfully located ACPI 2.0 RSDP
        }
    }
}

bool ValidateElf(axiom::Elf64_Ehdr* ehdr) {
    if (ehdr->e_ident[0] != axiom::ELFMAG0 ||
        ehdr->e_ident[1] != axiom::ELFMAG1 ||
        ehdr->e_ident[2] != axiom::ELFMAG2 ||
        ehdr->e_ident[3] != axiom::ELFMAG3) {
        return false;
    }
    if (ehdr->e_machine != axiom::EM_X86_64) return false;
    return true;
}

void LoadKernel(EFI_HANDLE ImageHandle) {
    EFI_FILE_PROTOCOL* kernelFile = OpenKernelFile(ImageHandle);
    if (!kernelFile) return;

    // Read ELF header
    axiom::Elf64_Ehdr ehdr;
    UINTN size = sizeof(axiom::Elf64_Ehdr);
    kernelFile->Read(kernelFile, &size, &ehdr);

    if (!ValidateElf(&ehdr)) return;

    // Iterate over Program Headers
    for (int i = 0; i < ehdr.e_phnum; i++) {
        axiom::Elf64_Phdr phdr;
        kernelFile->SetPosition(kernelFile, ehdr.e_phoff + i * ehdr.e_phentsize);
        size = sizeof(axiom::Elf64_Phdr);
        kernelFile->Read(kernelFile, &size, &phdr);

        if (phdr.p_type == axiom::PT_LOAD) {
            // Allocate memory and load segment
            // TODO: Use UEFI AllocatePages/AllocatePool
        }
    }
}

void TransitionToKernel(BootInfo* boot_info) {
    // TODO: Construct paging, jump to entry point
}

} // namespace axiom::sentinel

