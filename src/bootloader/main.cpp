#include "axiom/boot/uefi.hpp"
#include "axiom/boot/info.hpp"
#include "axiom/boot/elf.hpp"

using namespace axiom::uefi;
using namespace axiom;

// ====================================================================
// 1. Freestanding C/C++ Replacements
// ====================================================================
extern "C" {
void* memset(void* s, int c, size_t n) {
    unsigned char* p = static_cast<unsigned char*>(s);
    for (size_t i = 0; i < n; ++i) p[i] = static_cast<unsigned char>(c);
    return s;
}

void* memcpy(void* dest, const void* src, size_t n) {
    unsigned char* d = static_cast<unsigned char*>(dest);
    const unsigned char* s = static_cast<const unsigned char*>(src);
    for (size_t i = 0; i < n; ++i) d[i] = s[i];
    return dest;
}

int memcmp(const void* s1, const void* s2, size_t n) {
    const unsigned char* p1 = static_cast<const unsigned char*>(s1);
    const unsigned char* p2 = static_cast<const unsigned char*>(s2);
    for (size_t i = 0; i < n; ++i) {
        if (p1[i] != p2[i]) return p1[i] < p2[i] ? -1 : 1;
    }
    return 0;
}
} // extern "C"

// ====================================================================
// 2. Global State & GUIDs
// ====================================================================
namespace {

EFI_SYSTEM_TABLE* gST = nullptr;
EFI_BOOT_SERVICES_FULL* gBS = nullptr;

static const EFI_GUID EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID =
    {0x9042a9de, 0x23dc, 0x4a38, {0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a}};

static const EFI_GUID EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID =
    {0x964e5b22, 0x6459, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}};

static const EFI_GUID EFI_LOADED_IMAGE_PROTOCOL_GUID =
    {0x5B1B31A1, 0x9562, 0x11d2, {0x8E, 0x3F, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B}};

static const EFI_GUID ACPI_20_TABLE_GUID =
    {0x8868E871, 0xE4F1, 0x11D3, {0xBC, 0x22, 0x00, 0x80, 0xC7, 0x3C, 0x88, 0x81}};

// ====================================================================
// 3. Helpers
// ====================================================================
void Print(const int16_t* str) {
    if (gST && gST->ConOut) {
        gST->ConOut->OutputString(gST->ConOut, str);
    }
}

void PrintHex(uint64_t val) {
    int16_t buf[19];
    buf[0] = u'0';
    buf[1] = u'x';
    buf[18] = 0;
    for (int i = 17; i >= 2; --i) {
        int nibble = val & 0xF;
        if (nibble < 10) buf[i] = u'0' + nibble;
        else buf[i] = u'a' + (nibble - 10);
        val >>= 4;
    }
    Print(buf);
    Print((const int16_t*)u"\r\n");
}

[[noreturn]] void Panic(const int16_t* msg) {
    Print((const int16_t*)u"\r\n[BOOTLOADER PANIC] ");
    Print(msg);
    Print((const int16_t*)u"\r\n");
    while (true) __asm__ volatile("hlt");
}

EFI_STATUS GetFileHandle(EFI_HANDLE image_handle, const int16_t* path, EFI_FILE_PROTOCOL** out_file) {
    EFI_LOADED_IMAGE_PROTOCOL* loaded_image = nullptr;
    EFI_STATUS status = gBS->HandleProtocol(
        image_handle,
        const_cast<EFI_GUID*>(&EFI_LOADED_IMAGE_PROTOCOL_GUID),
        reinterpret_cast<void**>(&loaded_image)
    );
    if (status != EFI_SUCCESS) return status;

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fs = nullptr;
    status = gBS->HandleProtocol(
        loaded_image->DeviceHandle,
        const_cast<EFI_GUID*>(&EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID),
        reinterpret_cast<void**>(&fs)
    );
    if (status != EFI_SUCCESS) return status;

    EFI_FILE_PROTOCOL* root = nullptr;
    status = fs->OpenVolume(fs, &root);
    if (status != EFI_SUCCESS) return status;

    return root->Open(root, out_file, path, 0x01, 0); // Read-only
}

// Memory mapping definitions for Page Tables
constexpr uint64_t PAGE_SIZE = 4096;
constexpr uint64_t PAGE_PRESENT = 0b1;
constexpr uint64_t PAGE_RW = 0b10;
constexpr uint64_t PAGE_HUGE = 0b10000000;

void MapPage(uint64_t* pml4, uint64_t vaddr, uint64_t paddr, uint64_t flags) {
    uint64_t pml4_idx = (vaddr >> 39) & 0x1FF;
    uint64_t pdpt_idx = (vaddr >> 30) & 0x1FF;
    uint64_t pd_idx   = (vaddr >> 21) & 0x1FF;
    uint64_t pt_idx   = (vaddr >> 12) & 0x1FF;

    if (!(pml4[pml4_idx] & PAGE_PRESENT)) {
        EFI_PHYSICAL_ADDRESS new_pdpt;
        if (gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &new_pdpt) != EFI_SUCCESS) Panic((const int16_t*)u"Failed to allocate PDPT");
        memset((void*)new_pdpt, 0, PAGE_SIZE);
        pml4[pml4_idx] = (new_pdpt & ~(PAGE_SIZE - 1)) | PAGE_PRESENT | PAGE_RW;
    }

    uint64_t* pdpt = (uint64_t*)(pml4[pml4_idx] & ~(PAGE_SIZE - 1));

    if (!(pdpt[pdpt_idx] & PAGE_PRESENT)) {
        EFI_PHYSICAL_ADDRESS new_pd;
        if (gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &new_pd) != EFI_SUCCESS) Panic((const int16_t*)u"Failed to allocate PD");
        memset((void*)new_pd, 0, PAGE_SIZE);
        pdpt[pdpt_idx] = (new_pd & ~(PAGE_SIZE - 1)) | PAGE_PRESENT | PAGE_RW;
    }

    uint64_t* pd = (uint64_t*)(pdpt[pdpt_idx] & ~(PAGE_SIZE - 1));

    if (!(pd[pd_idx] & PAGE_PRESENT)) {
        EFI_PHYSICAL_ADDRESS new_pt;
        if (gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &new_pt) != EFI_SUCCESS) Panic((const int16_t*)u"Failed to allocate PT");
        memset((void*)new_pt, 0, PAGE_SIZE);
        pd[pd_idx] = (new_pt & ~(PAGE_SIZE - 1)) | PAGE_PRESENT | PAGE_RW;
    }

    uint64_t* pt = (uint64_t*)(pd[pd_idx] & ~(PAGE_SIZE - 1));
    pt[pt_idx] = (paddr & ~(PAGE_SIZE - 1)) | flags;
}

} // anonymous namespace

// ====================================================================
// 4. Main Entry Point
// ====================================================================
extern "C" EFI_STATUS EFIAPI efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE* system_table) {
    gST = system_table;
    gBS = system_table->BootServices;

    Print((const int16_t*)u"AxiomOS Bootloader Starting...\r\n");

    // 1. Setup GOP
    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop = nullptr;
    if (gBS->LocateProtocol(const_cast<EFI_GUID*>(&EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID), nullptr, reinterpret_cast<void**>(&gop)) != EFI_SUCCESS) {
        Panic((const int16_t*)u"GOP not found.");
    }

    // 2. Open Kernel File
    EFI_FILE_PROTOCOL* kernel_file = nullptr;
    if (GetFileHandle(image_handle, (const int16_t*)u"\\kernel", &kernel_file) != EFI_SUCCESS) {
        Panic((const int16_t*)u"Failed to open kernel file.");
    }

    // 3. Read ELF Header
    axiom::elf::Elf64_Ehdr ehdr;
    size_t ehdr_size = sizeof(ehdr);
    if (kernel_file->Read(kernel_file, &ehdr_size, &ehdr) != EFI_SUCCESS) {
        Panic((const int16_t*)u"Failed to read ELF header.");
    }

    if (memcmp(ehdr.e_ident, axiom::elf::ELFMAG, axiom::elf::SELFMAG) != 0) {
        Panic((const int16_t*)u"Invalid ELF magic.");
    }

    // 4. Allocate Page Tables (PML4)
    EFI_PHYSICAL_ADDRESS pml4_base;
    if (gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &pml4_base) != EFI_SUCCESS) {
        Panic((const int16_t*)u"Failed to allocate memory for PML4.");
    }
    memset((void*)pml4_base, 0, PAGE_SIZE);
    uint64_t* pml4 = (uint64_t*)pml4_base;

    // 5. Load PT_LOAD Segments
    for (int i = 0; i < ehdr.e_phnum; ++i) {
        axiom::elf::Elf64_Phdr phdr;
        size_t phdr_size = sizeof(phdr);
        kernel_file->SetPosition(kernel_file, ehdr.e_phoff + (i * ehdr.e_phentsize));
        kernel_file->Read(kernel_file, &phdr_size, &phdr);

        if (phdr.p_type == axiom::elf::PT_LOAD) {
            uint64_t vaddr = phdr.p_vaddr;
            uint64_t memsz = phdr.p_memsz;
            uint64_t filesz = phdr.p_filesz;
            uint64_t offset = phdr.p_offset;

            uint64_t vaddr_aligned = vaddr & ~(PAGE_SIZE - 1);
            uint64_t diff = vaddr - vaddr_aligned;
            size_t pages = (memsz + diff + PAGE_SIZE - 1) / PAGE_SIZE;

            EFI_PHYSICAL_ADDRESS alloc_addr;
            if (gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, pages, &alloc_addr) != EFI_SUCCESS) Panic((const int16_t*)u"Failed to allocate memory for ELF segment.");

            kernel_file->SetPosition(kernel_file, offset);
            kernel_file->Read(kernel_file, &filesz, (void*)(alloc_addr + diff));

            if (memsz > filesz) {
                memset((void*)(alloc_addr + diff + filesz), 0, memsz - filesz);
            }

            for (size_t p = 0; p < pages * PAGE_SIZE; p += PAGE_SIZE) {
                MapPage(pml4, vaddr_aligned + p, alloc_addr + p, PAGE_PRESENT | PAGE_RW);
                MapPage(pml4, alloc_addr + p, alloc_addr + p, PAGE_PRESENT | PAGE_RW);
            }
        }
    }

    kernel_file->Close(kernel_file);

    Print((const int16_t*)u"Kernel segments loaded and mapped. Constructing BootInfo...\r\n");

    // 6. Construct BootInfo
    EFI_PHYSICAL_ADDRESS bootinfo_addr;
    if (gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &bootinfo_addr) != EFI_SUCCESS) Panic((const int16_t*)u"Failed to allocate BootInfo.");
    BootInfo* bootinfo = (BootInfo*)bootinfo_addr;

    bootinfo->framebuffer.base = (void*)gop->Mode->frame_buffer_base;
    bootinfo->framebuffer.size = gop->Mode->frame_buffer_size;
    bootinfo->framebuffer.width = gop->Mode->info->horizontal_resolution;
    bootinfo->framebuffer.height = gop->Mode->info->vertical_resolution;
    bootinfo->framebuffer.pixels_per_scanline = gop->Mode->info->pixels_per_scan_line;

    bootinfo->rsdp = nullptr;
    for (size_t i = 0; i < gST->NumberOfTableEntries; ++i) {
        if (memcmp(&gST->ConfigurationTable[i].VendorGuid, &ACPI_20_TABLE_GUID, sizeof(EFI_GUID)) == 0) {
            bootinfo->rsdp = gST->ConfigurationTable[i].VendorTable;
            break;
        }
    }

    // 7. Map the first 4 GiB (Identity Map using 2MB huge pages)
    Print((const int16_t*)u"Identity mapping first 4GB...\r\n");
    for (uint64_t p = 0; p < 0x100000000; p += 0x200000) {
        uint64_t pml4_idx = (p >> 39) & 0x1FF;
        uint64_t pdpt_idx = (p >> 30) & 0x1FF;
        uint64_t pd_idx   = (p >> 21) & 0x1FF;

        if (!(pml4[pml4_idx] & PAGE_PRESENT)) {
            EFI_PHYSICAL_ADDRESS new_pdpt;
            gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &new_pdpt);
            memset((void*)new_pdpt, 0, PAGE_SIZE);
            pml4[pml4_idx] = (new_pdpt & ~(PAGE_SIZE - 1)) | PAGE_PRESENT | PAGE_RW;
        }

        uint64_t* pdpt = (uint64_t*)(pml4[pml4_idx] & ~(PAGE_SIZE - 1));

        if (!(pdpt[pdpt_idx] & PAGE_PRESENT)) {
            EFI_PHYSICAL_ADDRESS new_pd;
            gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &new_pd);
            memset((void*)new_pd, 0, PAGE_SIZE);
            pdpt[pdpt_idx] = (new_pd & ~(PAGE_SIZE - 1)) | PAGE_PRESENT | PAGE_RW;
        }

        uint64_t* pd = (uint64_t*)(pdpt[pdpt_idx] & ~(PAGE_SIZE - 1));
        pd[pd_idx] = (p & ~0x1FFFFF) | PAGE_PRESENT | PAGE_RW | PAGE_HUGE;
    }

    // 8. Final Handover
    Print((const int16_t*)u"ExitBootServices...\r\n");
    size_t map_size = 0;
    EFI_MEMORY_DESCRIPTOR* memory_map = nullptr;
    size_t map_key = 0;
    size_t desc_size = 0;
    uint32_t desc_version = 0;

    gBS->GetMemoryMap(&map_size, nullptr, &map_key, &desc_size, &desc_version);
    map_size += 2 * desc_size;
    gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, (map_size + PAGE_SIZE - 1) / PAGE_SIZE, (EFI_PHYSICAL_ADDRESS*)&memory_map);
    gBS->GetMemoryMap(&map_size, memory_map, &map_key, &desc_size, &desc_version);

    using KernelEntryType = void(__attribute__((sysv_abi)) *)(BootInfo*);
    KernelEntryType KernelEntry = (KernelEntryType) ehdr.e_entry;

    Print((const int16_t*)u"PML4 Base: ");
    PrintHex(pml4_base);
    Print((const int16_t*)u"Kernel Entry: ");
    PrintHex((uint64_t)KernelEntry);

    EFI_STATUS status = gBS->ExitBootServices(image_handle, map_key);
    if (status != EFI_SUCCESS) {
        gBS->GetMemoryMap(&map_size, memory_map, &map_key, &desc_size, &desc_version);
        status = gBS->ExitBootServices(image_handle, map_key);
        if (status != EFI_SUCCESS) Panic((const int16_t*)u"ExitBootServices failed on retry");
    }

    // Load the NEW PML4 into CR3 to activate higher-half kernel mapping
    __asm__ volatile("mov %0, %%cr3" : : "r"(pml4_base));

    KernelEntry(bootinfo);

    while(true) { __asm__ volatile("cli; hlt"); }
    return EFI_SUCCESS;
}
