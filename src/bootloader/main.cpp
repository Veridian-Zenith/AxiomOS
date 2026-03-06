// Module description: AxiomOS UEFI Bootloader.
// Architecture: x86_64
// Safety: This code runs in the UEFI environment and is responsible for
//         the initial hardware setup, loading the kernel, and transitioning
//         the system to a state suitable for the kernel to take over.
#include "axiomos/bootinfo.hpp"
#include "axiomos/uefi.hpp"
#include "axiomos/elf.hpp"
#include <stddef.h>
#include <stdint.h>

using namespace axiom::uefi;
using namespace axiom::elf;

// ============================================================================
// GUID Definitions
// ============================================================================

static const EFI_GUID EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID =
    {0x9042a9de, 0x23dc, 0x4a38, {0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a}};

static const EFI_GUID EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID =
    {0x964e5b22, 0x6459, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}};

static const EFI_GUID EFI_LOADED_IMAGE_PROTOCOL_GUID =
    {0x5B1B31A1, 0x9562, 0x11d2, {0x8E, 0x3F, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B}};

// ============================================================================
// Minimal Standard Library for Freestanding Environment
// ============================================================================

extern "C" {
    void* memcpy(void* dest, const void* src, size_t n) {
        char* d = (char*)dest;
        const char* s = (const char*)src;
        for (size_t i = 0; i < n; i++) d[i] = s[i];
        return dest;
    }

    void* memset(void* s, int c, size_t n) {
        unsigned char* p = (unsigned char*)s;
        for (size_t i = 0; i < n; i++) p[i] = (unsigned char)c;
        return s;
    }

    int memcmp(const void* s1, const void* s2, size_t n) {
        const unsigned char* p1 = (const unsigned char*)s1;
        const unsigned char* p2 = (const unsigned char*)s2;
        for (size_t i = 0; i < n; i++) {
            if (p1[i] != p2[i]) return p1[i] - p2[i];
        }
        return 0;
    }
}

// ============================================================================
// Page Table Structures
// ============================================================================

struct PageTableEntry {
    uint64_t present : 1;
    uint64_t rw : 1;
    uint64_t user : 1;
    uint64_t pwt : 1;
    uint64_t pcd : 1;
    uint64_t accessed : 1;
    uint64_t dirty : 1;
    uint64_t huge : 1; // 1 for 2MB/1GB pages
    uint64_t global : 1;
    uint64_t available : 3;
    uint64_t address : 40;
    uint64_t available2 : 11;
    uint64_t nx : 1;
} __attribute__((packed));

struct PageTable {
    PageTableEntry entries[512];
} __attribute__((aligned(4096)));

// ============================================================================
// Bootloader State
// ============================================================================

EFI_SYSTEM_TABLE* gST = nullptr;
EFI_BOOT_SERVICES* gBS = nullptr;

void Print(const int16_t* str) {
    if (gST && gST->ConsoleOut) {
        gST->ConsoleOut->OutputString(gST->ConsoleOut, str);
    }
}

void PrintHex(uint64_t value) {
    int16_t buffer[20];
    int idx = 0;
    buffer[idx++] = '0';
    buffer[idx++] = 'x';

    for (int i = 15; i >= 0; i--) {
        int nibble = (value >> (i * 4)) & 0xF;
        if (nibble < 10) buffer[idx++] = '0' + nibble;
        else buffer[idx++] = 'A' + (nibble - 10);
    }
    buffer[idx++] = '\r';
    buffer[idx++] = '\n';
    buffer[idx] = 0;
    Print(buffer);
}

// ============================================================================
// Helper Functions
// ============================================================================

EFI_STATUS GetFileHandle(EFI_HANDLE image_handle, const int16_t* path, EFI_FILE_PROTOCOL** out_file) {
    EFI_STATUS status;

    // 1. Get Loaded Image Protocol to find the device handle
    void* loaded_image_void = nullptr;
    status = gBS->HandleProtocol(image_handle, (EFI_GUID*)&EFI_LOADED_IMAGE_PROTOCOL_GUID, &loaded_image_void);
    if (status != EFI_SUCCESS) return status;

    // We have to cast void* to a struct that has DeviceHandle as the first member or define LoadedImage protocol.
    // Hack: LoadedImageProtocol's first two members are Revision and ParentHandle, then SystemTable, then DeviceHandle.
    // Let's define a minimal struct or interpret memory carefully.
    // Actually, let's just create a struct locally.
    struct EFI_LOADED_IMAGE_PROTOCOL_MIN {
        uint32_t Revision;
        EFI_HANDLE ParentHandle;
        EFI_SYSTEM_TABLE* SystemTable;
        EFI_HANDLE DeviceHandle;
    };
    auto* loaded_image = (EFI_LOADED_IMAGE_PROTOCOL_MIN*)loaded_image_void;

    // 2. Open Simple File System on that device
    void* fs_void = nullptr;
    status = gBS->HandleProtocol(loaded_image->DeviceHandle, (EFI_GUID*)&EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID, &fs_void);
    if (status != EFI_SUCCESS) return status;

    auto* fs = (EFI_SIMPLE_FILE_SYSTEM_PROTOCOL*)fs_void;

    // 3. Open Volume
    EFI_FILE_PROTOCOL* root = nullptr;
    status = fs->OpenVolume(fs, &root);
    if (status != EFI_SUCCESS) return status;

    // 4. Open File
    status = root->Open(root, out_file, path, EFI_FILE_MODE_READ, 0);
    return status;
}

// ============================================================================
// ELF Loader
// ============================================================================

Elf64_Phdr* gKernelPhdrs = nullptr;
uint16_t gKernelPhdrCount = 0;
uint64_t gKernelEntry = 0;

// We use an extern C linkage for the main UEFI entry point.
// We target PE/COFF format (x86_64-unknown-windows triple) for UEFI compatibility.
extern "C" {

EFI_STATUS efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE* system_table) {
    gST = system_table;
    gBS = system_table->BootServices;

    // Clear screen
    if (gST->ConsoleOut->ClearScreen) {
        gST->ConsoleOut->ClearScreen(gST->ConsoleOut);
    }

    Print((const int16_t*)u"AxiomOS Bootloader Starting...\r\n");

    // 1. Initialize GOP
    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop = nullptr;
    EFI_STATUS status = gBS->LocateProtocol((EFI_GUID*)&EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID, nullptr, (void**)&gop);
    if (status != EFI_SUCCESS) {
        Print((const int16_t*)u"GOP Not Found!\r\n");
        return status;
    }

    Print((const int16_t*)u"GOP Initialized\r\n");

    // 2. Load Kernel File
    EFI_FILE_PROTOCOL* kernel_file = nullptr;
    status = GetFileHandle(image_handle, (const int16_t*)u"kernel", &kernel_file);
    if (status != EFI_SUCCESS) {
        Print((const int16_t*)u"Kernel file not found! (Tried 'kernel')\r\n");
        return status;
    }

    // Read ELF header to find entry and segments
    Elf64_Ehdr ehdr;
    uint64_t size = sizeof(ehdr);
    kernel_file->Read(kernel_file, &size, &ehdr);
    gKernelEntry = ehdr.e_entry;
    gKernelPhdrCount = ehdr.e_phnum;

    // Read Program Headers for storage
    uint64_t phdr_total_size = ehdr.e_phnum * ehdr.e_phentsize;
    gBS->AllocatePool(EfiLoaderData, phdr_total_size, (void**)&gKernelPhdrs);
    kernel_file->SetPosition(kernel_file, ehdr.e_phoff);
    kernel_file->Read(kernel_file, &phdr_total_size, gKernelPhdrs);

    // Load Segments
    Print((const int16_t*)u"Loading Kernel Segments...\r\n");
    for (int i = 0; i < gKernelPhdrCount; i++) {
        if (gKernelPhdrs[i].p_type == PT_LOAD) {
            uint64_t mem_size = gKernelPhdrs[i].p_memsz;
            uint64_t pages = (mem_size + 4095) / 4096;
            EFI_PHYSICAL_ADDRESS phys_addr = 0;

            status = gBS->AllocatePages(AllocateAnyPages, EfiLoaderCode, pages, &phys_addr);
            if (status != EFI_SUCCESS) {
                Print((const int16_t*)u"Allocation failed\r\n");
                while(1);
            }

            // Store the physical address in the phdr so we know where we put it
            // (Original p_paddr might be 0 or virtual, we overwrite it with actual physical)
            gKernelPhdrs[i].p_paddr = phys_addr;

            // Zero memory
            memset((void*)phys_addr, 0, mem_size);

            // Load data
            kernel_file->SetPosition(kernel_file, gKernelPhdrs[i].p_offset);
            uint64_t file_size = gKernelPhdrs[i].p_filesz;
            kernel_file->Read(kernel_file, &file_size, (void*)phys_addr);

            PrintHex(gKernelPhdrs[i].p_vaddr);
            Print((const int16_t*)u" mapped to ");
            PrintHex(phys_addr);
        }
    }
    kernel_file->Close(kernel_file);

    // 3. Prepare Page Tables
    // We need a PML4, a PDPT, and a PD/PT.
    // We will identity map the first 4GB (covering UEFI and our load addresses)
    // And map the kernel higher half.

    EFI_PHYSICAL_ADDRESS pml4_phys = 0;
    gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &pml4_phys);
    PageTable* pml4 = (PageTable*)pml4_phys;
    memset(pml4, 0, sizeof(PageTable));

    // Identity Map (0-4GB) using 1GB pages if possible, or 2MB pages
    // For simplicity, let's use a PDPT for the first 512GB entry (Index 0 of PML4)
    EFI_PHYSICAL_ADDRESS pdpt_low_phys = 0;
    gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &pdpt_low_phys);
    PageTable* pdpt_low = (PageTable*)pdpt_low_phys;
    memset(pdpt_low, 0, sizeof(PageTable));

    pml4->entries[0].present = 1;
    pml4->entries[0].rw = 1;
    pml4->entries[0].address = pdpt_low_phys >> 12;

    // Map first 4GB using 2MB pages
    for (int i = 0; i < 4; i++) {
        EFI_PHYSICAL_ADDRESS pd_phys = 0;
        gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &pd_phys);
        PageTable* pd = (PageTable*)pd_phys;
        memset(pd, 0, sizeof(PageTable));

        pdpt_low->entries[i].present = 1;
        pdpt_low->entries[i].rw = 1;
        pdpt_low->entries[i].address = pd_phys >> 12;

        for (int j = 0; j < 512; j++) {
            pd->entries[j].present = 1;
            pd->entries[j].rw = 1;
            pd->entries[j].huge = 1; // 2MB page
            pd->entries[j].address = (uint64_t)((i * 512 + j) * 0x200000) >> 12;
        }
    }

    // High Map (Kernel)
    // Kernel vaddr: 0xFFFFFFFF80000000
    // PML4 Index: 511
    // PDPT Index: 510
    // PD Index: 0 (for 80000000) onwards

    // We need a PDPT for PML4[511]
    EFI_PHYSICAL_ADDRESS pdpt_high_phys = 0;
    gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &pdpt_high_phys);
    PageTable* pdpt_high = (PageTable*)pdpt_high_phys;
    memset(pdpt_high, 0, sizeof(PageTable));

    pml4->entries[511].present = 1;
    pml4->entries[511].rw = 1;
    pml4->entries[511].address = pdpt_high_phys >> 12;

    // We need a PD for PDPT[510]
    EFI_PHYSICAL_ADDRESS pd_high_phys = 0;
    gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &pd_high_phys);
    PageTable* pd_high = (PageTable*)pd_high_phys;
    memset(pd_high, 0, sizeof(PageTable));

    pdpt_high->entries[510].present = 1;
    pdpt_high->entries[510].rw = 1;
    pdpt_high->entries[510].address = pd_high_phys >> 12;

    // Now map the kernel segments
    for (int i = 0; i < gKernelPhdrCount; i++) {
        if (gKernelPhdrs[i].p_type == PT_LOAD) {
            uint64_t vaddr = gKernelPhdrs[i].p_vaddr;
            uint64_t paddr = gKernelPhdrs[i].p_paddr; // This is the REAL physical address we allocated
            uint64_t size = gKernelPhdrs[i].p_memsz;

            uint64_t start = vaddr;
            uint64_t end = vaddr + size;

            for (uint64_t cur = start; cur < end; cur += 4096) {
                // Calculate indices relative to 0xFFFFFFFF80000000
                uint64_t offset = cur - 0xFFFFFFFF80000000ULL;
                uint64_t pd_idx = (offset >> 21) & 0x1FF;
                uint64_t pt_idx = (offset >> 12) & 0x1FF;

                // Allocate PT if not present
                if (pd_high->entries[pd_idx].present == 0) {
                    EFI_PHYSICAL_ADDRESS pt_phys = 0;
                    gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &pt_phys);
                    memset((void*)pt_phys, 0, 4096);

                    pd_high->entries[pd_idx].present = 1;
                    pd_high->entries[pd_idx].rw = 1;
                    pd_high->entries[pd_idx].address = pt_phys >> 12;
                }

                PageTable* pt = (PageTable*)((uint64_t)pd_high->entries[pd_idx].address << 12);

                uint64_t page_offset = cur - start;
                uint64_t page_phys = paddr + page_offset;

                pt->entries[pt_idx].present = 1;
                pt->entries[pt_idx].rw = 1;
                pt->entries[pt_idx].address = page_phys >> 12;
            }
        }
    }

    // 4. Memory Map
    uint64_t map_size = 0;
    uint64_t map_key = 0;
    uint64_t desc_size = 0;
    uint32_t desc_version = 0;

    gBS->GetMemoryMap(&map_size, nullptr, &map_key, &desc_size, &desc_version);
    map_size += 2 * 4096; // Add some slack

    void* map_buffer = nullptr;
    gBS->AllocatePool(EfiLoaderData, map_size, &map_buffer);

    status = gBS->GetMemoryMap(&map_size, map_buffer, &map_key, &desc_size, &desc_version);
    if (status != EFI_SUCCESS) {
        Print((const int16_t*)u"Failed to get memory map\r\n");
        while(1);
    }

    // 5. Exit Boot Services
    status = gBS->ExitBootServices(image_handle, map_key);
    if (status != EFI_SUCCESS) {
        // Retry logic as per spec (map key might have changed)
        gBS->GetMemoryMap(&map_size, map_buffer, &map_key, &desc_size, &desc_version);
        gBS->ExitBootServices(image_handle, map_key);
    }

    // 6. Switch Page Tables
    // Load CR3
    asm volatile("mov %0, %%cr3" : : "r"(pml4_phys));

    // 7. Prepare BootInfo
    axiom::BootInfo boot_info;
    boot_info.framebuffer_base = gop->Mode->FrameBufferBase;
    boot_info.framebuffer_size = gop->Mode->FrameBufferSize;
    boot_info.framebuffer_width = gop->Mode->Info->HorizontalResolution;
    boot_info.framebuffer_height = gop->Mode->Info->VerticalResolution;
    boot_info.framebuffer_stride = gop->Mode->Info->PixelsPerScanLine;

    // Determine format
    if (gop->Mode->Info->PixelFormat == 1) { // PixelRedGreenBlueReserved8BitPerColor
        boot_info.framebuffer_format = 1;
    } else if (gop->Mode->Info->PixelFormat == 2) { // PixelBlueGreenRedReserved8BitPerColor
        boot_info.framebuffer_format = 2;
    } else {
        boot_info.framebuffer_format = 0;
    }

    boot_info.efi_memory_map = (axiom::EfiMemoryDescriptor*)map_buffer;
    boot_info.efi_memory_map_size = map_size;
    boot_info.efi_descriptor_size = desc_size;
    boot_info.efi_descriptor_version = desc_version;
    boot_info.kernel_entry = gKernelEntry;

    // 8. Jump to Kernel
    typedef void (*KernelEntry)(axiom::BootInfo*);
    KernelEntry kentry = (KernelEntry)gKernelEntry;

    kentry(&boot_info);

    while(1);
    return EFI_SUCCESS;
}

} // extern "C"
