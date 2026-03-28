/*
    OSL-3.0 License
    https://opensource.org/licenses/OSL-3.0
    Copyright (c) 2026 Veridian Zenith. All rights reserved.

    Module description:
    This header provides essential type definitions, GUIDs, and protocol
    structures required to interact with the UEFI environment. These definitions
    are based on the UEFI Specification and are necessary for building a
    freestanding UEFI application.
*/

#pragma once

#include <stddef.h>
#include <stdint.h>

#define EFIAPI __attribute__((ms_abi))

namespace axiom::uefi {

// --- Basic Types ------------------------------------------------------------
using EFI_HANDLE = void*;
using EFI_STATUS = size_t;
using EFI_PHYSICAL_ADDRESS = uint64_t;
using EFI_VIRTUAL_ADDRESS = uint64_t;

constexpr EFI_STATUS EFI_SUCCESS = 0;
#define EFIERR(a) (0x8000000000000000 | (a))
constexpr EFI_STATUS EFI_NOT_FOUND = EFIERR(14);
constexpr EFI_STATUS EFI_BUFFER_TOO_SMALL = EFIERR(5);

struct EFI_GUID {
    uint32_t data1;
    uint16_t data2;
    uint16_t data3;
    uint8_t  data4[8];
};

struct EFI_TABLE_HEADER {
    uint64_t Signature;
    uint32_t Revision;
    uint32_t HeaderSize;
    uint32_t CRC32;
    uint32_t Reserved;
};

// --- Protocols --------------------------------------------------------------
struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;
struct EFI_SYSTEM_TABLE;
struct EFI_BOOT_SERVICES;
struct EFI_RUNTIME_SERVICES;
struct EFI_GRAPHICS_OUTPUT_PROTOCOL;
struct EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;
struct EFI_FILE_PROTOCOL;
struct EFI_LOADED_IMAGE_PROTOCOL;

// --- EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL ----------------------------------------
using EFI_TEXT_STRING = EFI_STATUS (EFIAPI *)(
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* self,
    const int16_t* string
);

using EFI_TEXT_CLEAR_SCREEN = EFI_STATUS (EFIAPI *)(
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* self
);

struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
    uint64_t _unused;
    EFI_TEXT_STRING OutputString;
    uint64_t _unused2[4];
    EFI_TEXT_CLEAR_SCREEN ClearScreen;
};


// --- EFI_BOOT_SERVICES ------------------------------------------------------
enum EFI_ALLOCATE_TYPE {
    AllocateAnyPages,
    AllocateMaxAddress,
    AllocateAddress,
    MaxAllocateType
};

enum EFI_MEMORY_TYPE {
    EfiReservedMemoryType,
    EfiLoaderCode,
    EfiLoaderData,
    EfiBootServicesCode,
    EfiBootServicesData,
    EfiRuntimeServicesCode,
    EfiRuntimeServicesData,
    EfiConventionalMemory,
    EfiUnusableMemory,
    EfiACPIReclaimMemory,
    EfiACPIMemoryNVS,
    EfiMemoryMappedIO,
    EfiMemoryMappedIOPortSpace,
    EfiPalCode,
    EfiPersistentMemory,
    EfiUnacceptedMemoryType,
    EfiMaxMemoryType
};

struct EFI_MEMORY_DESCRIPTOR {
    uint32_t Type;
    uint32_t Padding;
    EFI_PHYSICAL_ADDRESS PhysicalStart;
    EFI_VIRTUAL_ADDRESS VirtualStart;
    uint64_t NumberOfPages;
    uint64_t Attribute;
};

using EFI_ALLOCATE_PAGES = EFI_STATUS (EFIAPI *)(
    EFI_ALLOCATE_TYPE Type,
    EFI_MEMORY_TYPE MemoryType,
    size_t Pages,
    EFI_PHYSICAL_ADDRESS* Memory
);

using EFI_FREE_PAGES = EFI_STATUS (EFIAPI *)(
    EFI_PHYSICAL_ADDRESS Memory,
    size_t Pages
);

using EFI_GET_MEMORY_MAP = EFI_STATUS (EFIAPI *)(
    size_t* MemoryMapSize,
    EFI_MEMORY_DESCRIPTOR* MemoryMap,
    size_t* MapKey,
    size_t* DescriptorSize,
    uint32_t* DescriptorVersion
);

using EFI_ALLOCATE_POOL = EFI_STATUS (EFIAPI *)(
    EFI_MEMORY_TYPE PoolType,
    size_t Size,
    void** Buffer
);

using EFI_FREE_POOL = EFI_STATUS (EFIAPI *)(
    void* Buffer
);

using EFI_HANDLE_PROTOCOL = EFI_STATUS (EFIAPI *)(
    EFI_HANDLE Handle,
    EFI_GUID* Protocol,
    void** Interface
);

using EFI_LOCATE_PROTOCOL = EFI_STATUS (EFIAPI *)(
    EFI_GUID* Protocol,
    void* Registration,
    void** Interface
);

using EFI_EXIT_BOOT_SERVICES = EFI_STATUS (EFIAPI *)(
    EFI_HANDLE ImageHandle,
    size_t MapKey
);

struct EFI_BOOT_SERVICES_FULL {
    EFI_TABLE_HEADER Hdr;

    void* RaiseTPL;
    void* RestoreTPL;

    EFI_ALLOCATE_PAGES AllocatePages;
    EFI_FREE_PAGES FreePages;
    EFI_GET_MEMORY_MAP GetMemoryMap;
    EFI_ALLOCATE_POOL AllocatePool;
    EFI_FREE_POOL FreePool;

    void* CreateEvent;
    void* SetTimer;
    void* WaitForEvent;
    void* SignalEvent;
    void* CloseEvent;
    void* CheckEvent;

    void* InstallProtocolInterface;
    void* ReinstallProtocolInterface;
    void* UninstallProtocolInterface;
    EFI_HANDLE_PROTOCOL HandleProtocol;
    void* Reserved;
    void* RegisterProtocolNotify;
    void* LocateHandle;
    void* LocateDevicePath;
    void* InstallConfigurationTable;

    void* LoadImage;
    void* StartImage;
    void* Exit;
    void* UnloadImage;
    EFI_EXIT_BOOT_SERVICES ExitBootServices;

    void* GetNextMonotonicCount;
    void* Stall;
    void* SetWatchdogTimer;

    void* ConnectController;
    void* DisconnectController;

    void* OpenProtocol;
    void* CloseProtocol;
    void* OpenProtocolInformation;

    void* ProtocolsPerHandle;
    void* LocateHandleBuffer;
    EFI_LOCATE_PROTOCOL LocateProtocol;
    void* InstallMultipleProtocolInterfaces;
    void* UninstallMultipleProtocolInterfaces;

    void* CalculateCrc32;

    void* CopyMem;
    void* SetMem;
    void* CreateEventEx;
};


struct EFI_CONFIGURATION_TABLE {
    EFI_GUID VendorGuid;
    void* VendorTable;
};

// --- EFI_SYSTEM_TABLE -------------------------------------------------------
struct EFI_SYSTEM_TABLE {
    EFI_TABLE_HEADER Hdr;
    const int16_t* FirmwareVendor;
    uint32_t FirmwareRevision;
    EFI_HANDLE ConsoleInHandle;
    void* ConIn;
    EFI_HANDLE ConsoleOutHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* ConOut;
    EFI_HANDLE StandardErrorHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* StdErr;
    EFI_RUNTIME_SERVICES* RuntimeServices;
    EFI_BOOT_SERVICES_FULL* BootServices;
    size_t NumberOfTableEntries;
    EFI_CONFIGURATION_TABLE* ConfigurationTable;
};


// --- EFI_GRAPHICS_OUTPUT_PROTOCOL (GOP) -------------------------------------
struct EFI_PIXEL_BITMASK {
    uint32_t red_mask;
    uint32_t green_mask;
    uint32_t blue_mask;
    uint32_t reserved_mask;
};

enum EFI_GRAPHICS_PIXEL_FORMAT {
    PixelRedGreenBlueReserved8BitPerColor,
    PixelBlueGreenRedReserved8BitPerColor,
    PixelBitMask,
    PixelBltOnly,
    PixelFormatMax
};

struct EFI_GRAPHICS_OUTPUT_MODE_INFORMATION {
    uint32_t version;
    uint32_t horizontal_resolution;
    uint32_t vertical_resolution;
    EFI_GRAPHICS_PIXEL_FORMAT pixel_format;
    EFI_PIXEL_BITMASK pixel_information;
    uint32_t pixels_per_scan_line;
};

struct EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE {
    uint32_t max_mode;
    uint32_t mode;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* info;
    size_t size_of_info;
    EFI_PHYSICAL_ADDRESS frame_buffer_base;
    size_t frame_buffer_size;
};

struct EFI_GRAPHICS_OUTPUT_PROTOCOL {
    void* QueryMode;
    void* SetMode;
    void* Blt;
    EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* Mode;
};

// --- EFI File Protocols -----------------------------------------------------
struct EFI_FILE_PROTOCOL {
    uint64_t Revision;
    EFI_STATUS (EFIAPI *Open)(
        EFI_FILE_PROTOCOL* self,
        EFI_FILE_PROTOCOL** new_handle,
        const int16_t* file_name,
        uint64_t open_mode,
        uint64_t attributes
    );
    EFI_STATUS (EFIAPI *Close)(EFI_FILE_PROTOCOL* self);
    EFI_STATUS (EFIAPI *Delete)(EFI_FILE_PROTOCOL* self);
    EFI_STATUS (EFIAPI *Read)(
        EFI_FILE_PROTOCOL* self,
        size_t* buffer_size,
        void* buffer
    );
    EFI_STATUS (EFIAPI *Write)(
        EFI_FILE_PROTOCOL* self,
        size_t* buffer_size,
        void* buffer
    );
    EFI_STATUS (EFIAPI *SetPosition)(
        EFI_FILE_PROTOCOL* self,
        uint64_t position
    );
    EFI_STATUS (EFIAPI *GetPosition)(
        EFI_FILE_PROTOCOL* self,
        uint64_t* position
    );
    EFI_STATUS (EFIAPI *GetInfo)(
        EFI_FILE_PROTOCOL* self,
        EFI_GUID* information_type,
        size_t* buffer_size,
        void* buffer
    );
    EFI_STATUS (EFIAPI *SetInfo)(
        EFI_FILE_PROTOCOL* self,
        EFI_GUID* information_type,
        size_t buffer_size,
        void* buffer
    );
    EFI_STATUS (EFIAPI *Flush)(EFI_FILE_PROTOCOL* self);
};

struct EFI_SIMPLE_FILE_SYSTEM_PROTOCOL {
    uint64_t Revision;
    EFI_STATUS (EFIAPI *OpenVolume)(
        EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* self,
        EFI_FILE_PROTOCOL** root
    );
};

// --- EFI_LOADED_IMAGE_PROTOCOL ----------------------------------------------
struct EFI_LOADED_IMAGE_PROTOCOL {
    uint32_t Revision;
    EFI_HANDLE ParentHandle;
    EFI_SYSTEM_TABLE* SystemTable;
    EFI_HANDLE DeviceHandle;
    void* FilePath;
    void* Reserved;
    uint32_t LoadOptionsSize;
    void* LoadOptions;
    void* ImageBase;
    uint64_t ImageSize;
    uint64_t ImageCodeType;
    uint64_t ImageDataType;
    void* Unload;
};


} // namespace axiom::uefi
