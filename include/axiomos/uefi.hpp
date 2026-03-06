#ifndef AXIOMOS_UEFI_HPP
#define AXIOMOS_UEFI_HPP

#include <stdint.h>

namespace axiom::uefi {

    typedef uint64_t EFI_STATUS;
    typedef void* EFI_HANDLE;
    typedef uint64_t EFI_PHYSICAL_ADDRESS;
    typedef uint64_t EFI_VIRTUAL_ADDRESS;

    constexpr EFI_STATUS EFI_SUCCESS = 0;
    constexpr EFI_STATUS EFI_LOAD_ERROR = 1;
    constexpr EFI_STATUS EFI_INVALID_PARAMETER = 2;
    constexpr EFI_STATUS EFI_UNSUPPORTED = 3;
    constexpr EFI_STATUS EFI_BAD_BUFFER_SIZE = 4;
    constexpr EFI_STATUS EFI_BUFFER_TOO_SMALL = 5;
    constexpr EFI_STATUS EFI_NOT_FOUND = 14;

    // GUID structure
    struct EFI_GUID {
        uint32_t Data1;
        uint16_t Data2;
        uint16_t Data3;
        uint8_t  Data4[8];
    };

    // Table Header
    struct EFI_TABLE_HEADER {
        uint64_t Signature;
        uint32_t Revision;
        uint32_t HeaderSize;
        uint32_t CRC32;
        uint32_t Reserved;
    };

    // Memory Types
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
        EfiMaxMemoryType
    };

    enum EFI_ALLOCATE_TYPE {
        AllocateAnyPages,
        AllocateMaxAddress,
        AllocateAddress,
        MaxAllocateType
    };

    // Text Output Protocol
    struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

    typedef EFI_STATUS (*EFI_TEXT_STRING)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This, const int16_t* String);
    typedef EFI_STATUS (*EFI_TEXT_CLEAR_SCREEN)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This);

    struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
        void* Reset;
        EFI_TEXT_STRING OutputString;
        void* TestString;
        void* QueryMode;
        void* SetMode;
        void* SetAttribute;
        EFI_TEXT_CLEAR_SCREEN ClearScreen;
        void* SetCursorPosition;
        void* EnableCursor;
        void* Mode;
    };

    // File System Protocol
    struct EFI_FILE_PROTOCOL;

    typedef EFI_STATUS (*EFI_FILE_OPEN)(EFI_FILE_PROTOCOL* This, EFI_FILE_PROTOCOL** NewHandle, const int16_t* FileName, uint64_t OpenMode, uint64_t Attributes);
    typedef EFI_STATUS (*EFI_FILE_CLOSE)(EFI_FILE_PROTOCOL* This);
    typedef EFI_STATUS (*EFI_FILE_READ)(EFI_FILE_PROTOCOL* This, uint64_t* BufferSize, void* Buffer);
    typedef EFI_STATUS (*EFI_FILE_GET_INFO)(EFI_FILE_PROTOCOL* This, EFI_GUID* InformationType, uint64_t* BufferSize, void* Buffer);

    struct EFI_FILE_PROTOCOL {
        uint64_t Revision;
        EFI_FILE_OPEN Open;
        EFI_FILE_CLOSE Close;
        void* Delete;
        EFI_FILE_READ Read;
        void* Write;
        void* GetPosition;
        EFI_STATUS (*SetPosition)(EFI_FILE_PROTOCOL* This, uint64_t Position);
        EFI_FILE_GET_INFO GetInfo;
        void* SetInfo;
        void* Flush;
    };

    struct EFI_SIMPLE_FILE_SYSTEM_PROTOCOL {
        uint64_t Revision;
        EFI_STATUS (*OpenVolume)(EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* This, EFI_FILE_PROTOCOL** Root);
    };

    constexpr uint64_t EFI_FILE_MODE_READ = 0x0000000000000001;

    struct EFI_FILE_INFO {
        uint64_t Size;
        uint64_t FileSize;
        uint64_t PhysicalSize;
        void* CreateTime;
        void* LastAccessTime;
        void* ModificationTime;
        uint64_t Attribute;
        int16_t FileName[1];
    };

    // Graphics Output Protocol
    struct EFI_GRAPHICS_OUTPUT_MODE_INFORMATION {
        uint32_t Version;
        uint32_t HorizontalResolution;
        uint32_t VerticalResolution;
        uint32_t PixelFormat;
        uint32_t PixelInformation_RedMask;
        uint32_t PixelInformation_GreenMask;
        uint32_t PixelInformation_BlueMask;
        uint32_t PixelInformation_ReservedMask;
        uint32_t PixelsPerScanLine;
    };

    struct EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE {
        uint32_t MaxMode;
        uint32_t Mode;
        EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* Info;
        uint64_t SizeOfInfo;
        uint64_t FrameBufferBase;
        uint64_t FrameBufferSize;
    };

    struct EFI_GRAPHICS_OUTPUT_PROTOCOL {
        void* QueryMode;
        void* SetMode;
        void* Blt;
        EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* Mode;
    };

    // Boot Services
    struct EFI_BOOT_SERVICES {
        EFI_TABLE_HEADER Hdr;
        void* RaiseTPL;
        void* RestoreTPL;
        EFI_STATUS (*AllocatePages)(EFI_ALLOCATE_TYPE Type, EFI_MEMORY_TYPE MemoryType, uint64_t Pages, EFI_PHYSICAL_ADDRESS* Memory);
        EFI_STATUS (*FreePages)(EFI_PHYSICAL_ADDRESS Memory, uint64_t Pages);
        EFI_STATUS (*GetMemoryMap)(uint64_t* MemoryMapSize, void* MemoryMap, uint64_t* MapKey, uint64_t* DescriptorSize, uint32_t* DescriptorVersion);
        EFI_STATUS (*AllocatePool)(EFI_MEMORY_TYPE PoolType, uint64_t Size, void** Buffer);
        EFI_STATUS (*FreePool)(void* Buffer);
        void* CreateEvent;
        void* SetTimer;
        void* WaitForEvent;
        void* SignalEvent;
        void* CloseEvent;
        void* CheckEvent;
        void* InstallProtocolInterface;
        void* ReinstallProtocolInterface;
        void* UninstallProtocolInterface;
        EFI_STATUS (*HandleProtocol)(EFI_HANDLE Handle, EFI_GUID* Protocol, void** Interface);
        void* Reserved;
        void* RegisterProtocolNotify;
        void* LocateHandle;
        void* LocateDevicePath;
        void* InstallConfigurationTable;
        void* LoadImage;
        void* StartImage;
        EFI_STATUS (*Exit)(EFI_HANDLE ImageHandle, EFI_STATUS ExitStatus, uint64_t ExitDataSize, int16_t* ExitData);
        void* UnloadImage;
        EFI_STATUS (*ExitBootServices)(EFI_HANDLE ImageHandle, uint64_t MapKey);
        void* GetNextMonotonicCount;
        EFI_STATUS (*Stall)(uint64_t Microseconds);
        void* SetWatchdogTimer;
        void* ConnectController;
        void* DisconnectController;
        void* OpenProtocol;
        void* CloseProtocol;
        void* OpenProtocolInformation;
        void* ProtocolsPerHandle;
        void* LocateHandleBuffer;
        EFI_STATUS (*LocateProtocol)(EFI_GUID* Protocol, void* Registration, void** Interface);
        void* InstallMultipleProtocolInterfaces;
        void* UninstallMultipleProtocolInterfaces;
        void* CalculateCrc32;
        void* CopyMem;
        void* SetMem;
        void* CreateEventEx;
    };

    // System Table
    struct EFI_SYSTEM_TABLE {
        EFI_TABLE_HEADER Hdr;
        int16_t* FirmwareVendor;
        uint32_t FirmwareRevision;
        EFI_HANDLE ConsoleInHandle;
        void* ConsoleIn;
        EFI_HANDLE ConsoleOutHandle;
        EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* ConsoleOut;
        EFI_HANDLE ConsoleErrorHandle;
        EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* ConsoleError;
        void* RuntimeServices;
        EFI_BOOT_SERVICES* BootServices;
        uint64_t NumberOfTableEntries;
        void* ConfigurationTable;
    };

} // namespace axiom::uefi

#endif // AXIOMOS_UEFI_HPP
