#include "axiom/boot/uefi.hpp"
#include "axiom/boot/info.hpp"
#include "axiom/boot/elf.hpp"

using namespace axiom::uefi;
using namespace axiom;

static inline uint64_t __readmsr(uint32_t msr) {
    uint32_t low, high;
    __asm__ volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((uint64_t)high << 32) | low;
}

static inline void __writemsr(uint32_t msr, uint64_t value) {
    uint32_t low = value & 0xFFFFFFFF;
    uint32_t high = value >> 32;
    __asm__ volatile("wrmsr" : : "a"(low), "d"(high), "c"(msr));
}


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
}

// ====================================================================
// 2. Protocols & State
// ====================================================================
namespace {
EFI_SYSTEM_TABLE* gST = nullptr;
EFI_BOOT_SERVICES_FULL* gBS = nullptr;

static const EFI_GUID EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID = {0x9042a9de, 0x23dc, 0x4a38, {0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a}};
static const EFI_GUID EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID = {0x964e5b22, 0x6459, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}};
static const EFI_GUID EFI_LOADED_IMAGE_PROTOCOL_GUID = {0x5B1B31A1, 0x9562, 0x11d2, {0x8E, 0x3F, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B}};
static const EFI_GUID ACPI_20_TABLE_GUID = {0x8868E871, 0xE4F1, 0x11D3, {0xBC, 0x22, 0x00, 0x80, 0xC7, 0x3C, 0x88, 0x81}};

// Paging Constants
constexpr uint64_t PAGE_SIZE = 4096;
constexpr uint64_t PAGE_PRESENT = 1ULL << 0;
constexpr uint64_t PAGE_RW = 1ULL << 1;
constexpr uint64_t PAGE_HUGE = 1ULL << 7;
#ifndef PAGE_NX
constexpr uint64_t PAGE_NX = 1ULL << 63;
#endif

void Print(const int16_t* str) { gST->ConOut->OutputString(gST->ConOut, str); }

void MapPage(uint64_t* pml4, uint64_t vaddr, uint64_t paddr, uint64_t flags) {
    uint64_t indices[4] = {
        (vaddr >> 39) & 0x1FF, // PML4
        (vaddr >> 30) & 0x1FF, // PDPT
        (vaddr >> 21) & 0x1FF, // PD
        (vaddr >> 12) & 0x1FF  // PT
    };

    uint64_t* current_table = pml4;
    for (int i = 0; i < 3; ++i) {
        if (!(current_table[indices[i]] & PAGE_PRESENT)) {
            EFI_PHYSICAL_ADDRESS next_table;
            gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &next_table);
            memset((void*)next_table, 0, PAGE_SIZE);
            current_table[indices[i]] = next_table | PAGE_PRESENT | PAGE_RW;
        }
        current_table = (uint64_t*)(current_table[indices[i]] & ~0xFFFULL);
    }
    current_table[indices[3]] = (paddr & ~0xFFFULL) | (flags & 0xFFF) | PAGE_PRESENT;
}

void MapKernel(uint64_t* pml4, axiom::elf::Elf64_Ehdr& ehdr, EFI_FILE_PROTOCOL* kernel_file) {
    size_t ph_size = ehdr.e_phnum * ehdr.e_phentsize;
    axiom::elf::Elf64_Phdr* phdrs = (axiom::elf::Elf64_Phdr*)0;
    gBS->AllocatePool(EfiLoaderData, ph_size, (void**)&phdrs);
    kernel_file->SetPosition(kernel_file, ehdr.e_phoff);
    kernel_file->Read(kernel_file, &ph_size, phdrs);

    for (int i = 0; i < ehdr.e_phnum; ++i) {
        axiom::elf::Elf64_Phdr& phdr = phdrs[i];

        if (phdr.p_type == axiom::elf::PT_LOAD) {
            size_t pages = (phdr.p_memsz + (phdr.p_vaddr & 0xFFF) + PAGE_SIZE - 1) / PAGE_SIZE;
            EFI_PHYSICAL_ADDRESS phys_addr;
            gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, pages, &phys_addr);

            kernel_file->SetPosition(kernel_file, phdr.p_offset);
            size_t filesz = phdr.p_filesz;
            kernel_file->Read(kernel_file, &filesz, (void*)(phys_addr + (phdr.p_vaddr & 0xFFF)));

            uint64_t flags = PAGE_PRESENT | PAGE_RW; // Temporary DEBUG: Make everything writable
            // if (phdr.p_flags & axiom::elf::PF_W) flags |= PAGE_RW;
            // if (!(phdr.p_flags & axiom::elf::PF_X)) flags |= PAGE_NX;

            uint64_t vaddr_offset = phdr.p_vaddr & (PAGE_SIZE - 1);
            uint64_t start_v = phdr.p_vaddr & ~(PAGE_SIZE - 1);
            uint64_t end_v = (phdr.p_vaddr + phdr.p_memsz + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

            for (uint64_t v = start_v; v < end_v; v += PAGE_SIZE) {
                uint64_t p_offset = v - start_v;
                uint64_t paddr = phys_addr + vaddr_offset + p_offset;
                MapPage(pml4, v, paddr, flags);
            }

            if (phdr.p_memsz > phdr.p_filesz) {
                memset((void*)(phys_addr + vaddr_offset + phdr.p_filesz), 0, phdr.p_memsz - phdr.p_filesz);
            }
        }
    }
    gBS->FreePool(phdrs);
}
}

// ====================================================================
// 3. ELF Loader & Kernel Handover
// ====================================================================
extern "C" EFI_STATUS EFIAPI efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE* system_table) {
    gST = system_table;
    gBS = system_table->BootServices;

    Print((const int16_t*)u"AxiomOS Bootloader: Initializing NVMe Stack...\r\n");

    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop = nullptr;
    gBS->LocateProtocol(const_cast<EFI_GUID*>(&EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID), nullptr, (void**)&gop);

    EFI_LOADED_IMAGE_PROTOCOL* loaded_image;
    gBS->HandleProtocol(image_handle, const_cast<EFI_GUID*>(&EFI_LOADED_IMAGE_PROTOCOL_GUID), (void**)&loaded_image);

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fs;
    gBS->HandleProtocol(loaded_image->DeviceHandle, const_cast<EFI_GUID*>(&EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID), (void**)&fs);

    EFI_FILE_PROTOCOL *root, *kernel_file;
    fs->OpenVolume(fs, &root);
    root->Open(root, &kernel_file, (const int16_t*)u"kernel", 0x01, 0);

    axiom::elf::Elf64_Ehdr ehdr;
    size_t sz = sizeof(ehdr);
    kernel_file->Read(kernel_file, &sz, &ehdr);

    EFI_PHYSICAL_ADDRESS pml4_base;
    gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &pml4_base);
    memset((void*)pml4_base, 0, PAGE_SIZE);
    uint64_t* pml4 = (uint64_t*)pml4_base;

    MapKernel(pml4, ehdr, kernel_file);

    // Set Recursive Paging Entry (Index 510)
    pml4[510] = pml4_base | PAGE_PRESENT | PAGE_RW;

    // Identity Map the entire 4GB space for the transition (Safe & Reliable)
    for (uint64_t p = 0; p < 0x100000000; p += 0x200000) {
        // Use 2MB pages for identity map to speed up bootloader setup
        uint64_t pml4_idx = (p >> 39) & 0x1FF;
        uint64_t pdpt_idx = (p >> 30) & 0x1FF;
        uint64_t pd_idx = (p >> 21) & 0x1FF;

        if (!(pml4[pml4_idx] & PAGE_PRESENT)) {
            EFI_PHYSICAL_ADDRESS pdpt;
            gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &pdpt);
            memset((void*)pdpt, 0, PAGE_SIZE);
            pml4[pml4_idx] = pdpt | PAGE_PRESENT | PAGE_RW;
        }
        uint64_t* pdpt = (uint64_t*)(pml4[pml4_idx] & ~0xFFFULL);
        if (!(pdpt[pdpt_idx] & PAGE_PRESENT)) {
            EFI_PHYSICAL_ADDRESS pd;
            gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &pd);
            memset((void*)pd, 0, PAGE_SIZE);
            pdpt[pdpt_idx] = pd | PAGE_PRESENT | PAGE_RW;
        }
        uint64_t* pd = (uint64_t*)(pdpt[pdpt_idx] & ~0xFFFULL);
        pd[pd_idx] = p | PAGE_PRESENT | PAGE_RW | PAGE_HUGE;
    }

    BootInfo* bootinfo;
    gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, (EFI_PHYSICAL_ADDRESS*)&bootinfo);
    memset(bootinfo, 0, sizeof(BootInfo));
    bootinfo->framebuffer.base = (void*)gop->Mode->frame_buffer_base;
    bootinfo->framebuffer.size = gop->Mode->frame_buffer_size;

    size_t map_size = 0, map_key = 0, desc_size = 0;
    uint32_t desc_ver = 0;
    gBS->GetMemoryMap(&map_size, nullptr, &map_key, &desc_size, &desc_ver);
    map_size += (2 * desc_size);
    gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, (map_size / PAGE_SIZE) + 1, (EFI_PHYSICAL_ADDRESS*)&bootinfo->mmap);
    gBS->GetMemoryMap(&map_size, (EFI_MEMORY_DESCRIPTOR*)bootinfo->mmap, &map_key, &desc_size, &desc_ver);
    bootinfo->mmap_size = map_size;
    bootinfo->mmap_desc_size = desc_size;

    gBS->ExitBootServices(image_handle, map_key);

    uint64_t efer = __readmsr(0xC0000080);
    efer |= (1 << 11);
    __writemsr(0xC0000080, efer);

    __asm__ volatile("mov %0, %%cr3" : : "r"(pml4_base));

    using KernelEntry = void(__attribute__((sysv_abi)) *)(BootInfo*);
    ((KernelEntry)ehdr.e_entry)(bootinfo);

    while (true) __asm__ volatile("cli; hlt");
    return EFI_SUCCESS;
}

