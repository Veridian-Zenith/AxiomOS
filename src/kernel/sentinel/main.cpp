#include "internal.h"
#include "memory.h"
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

    axiom::BootInfo bootInfo = {};
    bootInfo.Signature = 0x584941434F4D30; // 'AXIOM0'

    axiom::sentinel::InitializeFirmware(config, &bootInfo);
    auto loadResult = axiom::sentinel::LoadKernel(ImageHandle);

    bootInfo.KernelStackTop = (uint64_t)AllocatePages(2) + 8192; // 8KB stack

    axiom::sentinel::TransitionToKernel(&bootInfo, loadResult.EntryPoint, loadResult.Pml4, bootInfo.KernelStackTop);

    return EFI_SUCCESS;
}

namespace axiom::sentinel {

// ... (OpenKernelFile stays the same) ...

void InitializeFirmware(SentinelConfig config, BootInfo* boot_info) {
    if (config.EnableFramebuffer) {
        EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
        EFI_GRAPHICS_OUTPUT_PROTOCOL* gop = nullptr;

        EFI_STATUS status = gST->BootServices->LocateProtocol(&gopGuid, nullptr, (void**)&gop);
        if (!EFI_ERROR(status)) {
            boot_info->GraphicsInfo.FramebufferBase = gop->Mode->FrameBufferBase;
            boot_info->GraphicsInfo.Width = gop->Mode->Info->HorizontalResolution;
            boot_info->GraphicsInfo.Height = gop->Mode->Info->VerticalResolution;
            boot_info->GraphicsInfo.PixelsPerScanLine = gop->Mode->Info->PixelsPerScanLine;
            boot_info->GraphicsInfo.Format = gop->Mode->Info->PixelFormat;
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
            boot_info->AcpiRsdpAddress = (uint64_t)rsdp;
        }
    }

    // Memory Map
    UINTN memoryMapSize = 0;
    EFI_MEMORY_DESCRIPTOR* memoryMap = nullptr;
    UINTN mapKey = 0;
    UINTN descriptorSize = 0;
    UINT32 descriptorVersion = 0;
    gST->BootServices->GetMemoryMap(&memoryMapSize, nullptr, &mapKey, &descriptorSize, &descriptorVersion);

    // Allocate memory for map - we might need extra space
    memoryMap = (EFI_MEMORY_DESCRIPTOR*)AllocatePool(memoryMapSize + (10 * descriptorSize));
    gST->BootServices->GetMemoryMap(&memoryMapSize, memoryMap, &mapKey, &descriptorSize, &descriptorVersion);

    boot_info->MemoryMapAddress = (uint64_t)memoryMap;
    boot_info->MemoryMapSize = memoryMapSize;
    boot_info->DescriptorSize = descriptorSize;
}

// ... (ValidateElf stays the same) ...

KernelLoadResult LoadKernel(EFI_HANDLE ImageHandle) {
    EFI_FILE_PROTOCOL* kernelFile = OpenKernelFile(ImageHandle);
    if (!kernelFile) return {0, nullptr};

    // Read ELF header
    axiom::Elf64_Ehdr ehdr;
    UINTN size = sizeof(axiom::Elf64_Ehdr);
    kernelFile->Read(kernelFile, &size, &ehdr);

    if (!ValidateElf(&ehdr)) return {0, nullptr};

    axiom::PageTable* pml4 = (axiom::PageTable*)AllocatePage();

    // Iterate over Program Headers
    for (int i = 0; i < ehdr.e_phnum; i++) {
        axiom::Elf64_Phdr phdr;
        kernelFile->SetPosition(kernelFile, ehdr.e_phoff + i * ehdr.e_phentsize);
        size = sizeof(axiom::Elf64_Phdr);
        kernelFile->Read(kernelFile, &size, &phdr);

        if (phdr.p_type == axiom::PT_LOAD) {
            // Allocate memory and load segment
            UINTN pages = (phdr.p_memsz + 4095) / 4096;
            void* physicalAddress = AllocatePages(pages);

            kernelFile->SetPosition(kernelFile, phdr.p_offset);
            UINTN readSize = phdr.p_filesz;
            kernelFile->Read(kernelFile, &readSize, physicalAddress);

            // Map segment
            for (UINTN page = 0; page < pages; ++page) {
                MapPage(pml4, phdr.p_vaddr + page * 4096, (uint64_t)physicalAddress + page * 4096, 0x2);
            }
        }
    }
    return {ehdr.e_entry, pml4};
}

void TransitionToKernel(BootInfo* boot_info, uint64_t entry_point, axiom::PageTable* pml4, uint64_t stack_top) {
    __asm__ volatile (
        "mov %0, %%cr3\n"
        "mov %1, %%rsp\n"
        "mov %2, %%rdi\n"
        "jmp *%3\n"
        :
        : "r"(pml4), "r"(stack_top), "r"(boot_info), "r"(entry_point)
        : "memory"
    );
}

} // namespace axiom::sentinel




