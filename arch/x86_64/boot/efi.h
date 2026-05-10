
#pragma once
#include <stdint.h>

typedef uint8_t   UINT8;
typedef uint16_t  UINT16;
typedef uint32_t  UINT32;
typedef uint64_t  UINT64;
typedef int64_t   INT64;
typedef uint16_t  CHAR16;
typedef uintptr_t UINTN;
typedef void*     EFI_HANDLE;
typedef uint64_t  EFI_STATUS;
typedef uint64_t  EFI_PHYSICAL_ADDRESS;
typedef uint64_t  EFI_VIRTUAL_ADDRESS;

#define EFI_SUCCESS             0ULL
#define EFI_ERROR(x)            (0x8000000000000000ULL | (x))
#define EFI_LOAD_ERROR          EFI_ERROR(1)
#define EFI_NOT_FOUND           EFI_ERROR(14)
#define EFI_BUFFER_TOO_SMALL    EFI_ERROR(5)

#define EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL 0x00000001

typedef struct { UINT32 a; UINT16 b; UINT16 c; UINT8 d[8]; } EFI_GUID;

#define EFI_LOADED_IMAGE_PROTOCOL_GUID \
    { 0x5B1B31A1,0x9562,0x11d2,{0x8E,0x3F,0x00,0xA0,0xC9,0x69,0x72,0x3B} }
#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID \
    { 0x964E5B22,0x6459,0x11D2,{0x8E,0x39,0x00,0xA0,0xC9,0x69,0x72,0x3B} }
#define EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID \
    { 0x9042A9DE,0x23DC,0x4A38,{0x96,0xFB,0x7A,0xDE,0xD0,0x80,0x51,0x6A} }
#define EFI_ACPI_20_TABLE_GUID \
    { 0x8868E871,0xE4F1,0x11D3,{0xBC,0x22,0x00,0x80,0xC7,0x3C,0x88,0x81} }

typedef enum {
    EfiReservedMemoryType    = 0,
    EfiLoaderCode            = 1,
    EfiLoaderData            = 2,
    EfiBootServicesCode      = 3,
    EfiBootServicesData      = 4,
    EfiRuntimeServicesCode   = 5,
    EfiRuntimeServicesData   = 6,
    EfiConventionalMemory    = 7,
    EfiUnusableMemory        = 8,
    EfiACPIReclaimMemory     = 9,
    EfiACPIMemoryNVS         = 10,
    EfiMemoryMappedIO        = 11,
    EfiMemoryMappedIOPortSpace = 12,
    EfiPalCode               = 13,
    EfiMaxMemoryType         = 14,
} EFI_MEMORY_TYPE;

typedef enum {
    AllocateAnyPages,
    AllocateMaxAddress,
    AllocateAddress,
    MaxAllocateType,
} EFI_ALLOCATE_TYPE;

typedef struct {
    UINT32              Type;
    UINT32              Pad;
    EFI_PHYSICAL_ADDRESS PhysicalStart;
    EFI_VIRTUAL_ADDRESS  VirtualStart;
    UINT64              NumberOfPages;
    UINT64              Attribute;
} EFI_MEMORY_DESCRIPTOR;

typedef struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
    void*      Reset;
    EFI_STATUS (__attribute__((ms_abi)) *OutputString)(
        struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, CHAR16 *String);
    void *QueryMode, *SetMode, *SetAttribute, *ClearScreen;
    void *SetCursorPosition, *EnableCursor;
    void *Mode;
} EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

typedef struct _EFI_FILE_PROTOCOL {
    UINT64     Revision;
    EFI_STATUS (__attribute__((ms_abi)) *Open)(
        struct _EFI_FILE_PROTOCOL *This, struct _EFI_FILE_PROTOCOL **NewHandle,
        CHAR16 *FileName, UINT64 OpenMode, UINT64 Attributes);
    EFI_STATUS (__attribute__((ms_abi)) *Close)(struct _EFI_FILE_PROTOCOL *This);
    void       *Delete;
    EFI_STATUS (__attribute__((ms_abi)) *Read)(
        struct _EFI_FILE_PROTOCOL *This, UINTN *BufferSize, void *Buffer);
    void *Write, *GetPosition, *SetPosition, *GetInfo, *SetInfo, *Flush;
} EFI_FILE_PROTOCOL;

#define EFI_FILE_MODE_READ 0x0000000000000001ULL

typedef struct {
    UINT64      Revision;
    EFI_STATUS (__attribute__((ms_abi)) *OpenVolume)(
        void *This, EFI_FILE_PROTOCOL **Root);
} EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;

typedef struct {
    UINT32              Revision;
    EFI_HANDLE          ParentHandle;
    void               *SystemTable;
    EFI_HANDLE          DeviceHandle;
    void               *FilePath;
    void               *Reserved;
    UINT32              LoadOptionsSize;
    void               *LoadOptions;
    void               *ImageBase;
    UINT64              ImageSize;
    EFI_MEMORY_TYPE     ImageCodeType;
    EFI_MEMORY_TYPE     ImageDataType;
    void               *Unload;
} EFI_LOADED_IMAGE_PROTOCOL;

typedef struct {
    UINT32 MaxMode, Mode, Info, SizeOfInfo;
    UINT64 FrameBufferBase;
    UINTN  FrameBufferSize;
} EFI_GRAPHICS_OUTPUT_MODE_INFORMATION;

typedef struct {
    UINT32 HorizontalResolution;
    UINT32 VerticalResolution;
    UINT32 PixelFormat;
    UINT32 RedMask, GreenMask, BlueMask, ReservedMask;
    UINT32 PixelsPerScanLine;
} EFI_GOP_MODE_INFO;

typedef struct {
    void       *QueryMode, *SetMode, *Blt;
    struct {
        UINT32                    MaxMode, Mode;
        EFI_GOP_MODE_INFO        *Info;
        UINTN                     SizeOfInfo;
        EFI_PHYSICAL_ADDRESS      FrameBufferBase;
        UINTN                     FrameBufferSize;
    } *Mode;
} EFI_GRAPHICS_OUTPUT_PROTOCOL;

typedef struct {
    UINT64    NumberOfTableEntries;
    struct { EFI_GUID VendorGuid; void *VendorTable; } *ConftableArray;
} EFI_CONFIGURATION_TABLE_HOLDER;

typedef struct {
    void *pad[3];
    EFI_STATUS (__attribute__((ms_abi)) *GetTime)(void*,void*);
    void *pad2[7];
} EFI_RUNTIME_SERVICES;

typedef struct _EFI_BOOT_SERVICES {
    UINT8  pad[24];
    void  *RaiseTPL, *RestoreTPL;
    EFI_STATUS (__attribute__((ms_abi)) *AllocatePages)(
        EFI_ALLOCATE_TYPE, EFI_MEMORY_TYPE, UINTN, EFI_PHYSICAL_ADDRESS*);
    EFI_STATUS (__attribute__((ms_abi)) *FreePages)(EFI_PHYSICAL_ADDRESS, UINTN);
    EFI_STATUS (__attribute__((ms_abi)) *GetMemoryMap)(
        UINTN *MapSize, EFI_MEMORY_DESCRIPTOR *Map, UINTN *MapKey,
        UINTN *DescriptorSize, UINT32 *DescriptorVersion);
    EFI_STATUS (__attribute__((ms_abi)) *AllocatePool)(
        EFI_MEMORY_TYPE, UINTN, void**);
    EFI_STATUS (__attribute__((ms_abi)) *FreePool)(void*);
    void *pad3[9];
    EFI_STATUS (__attribute__((ms_abi)) *HandleProtocol)(
        EFI_HANDLE, EFI_GUID*, void**);
    void *pad4[3];
    EFI_STATUS (__attribute__((ms_abi)) *LocateProtocol)(
        EFI_GUID*, void*, void**);
    void *pad5[5];
    EFI_STATUS (__attribute__((ms_abi)) *ExitBootServices)(EFI_HANDLE, UINTN);
} EFI_BOOT_SERVICES;

typedef struct {
    UINT8                          pad[44];
    EFI_HANDLE                     ConsoleInHandle;
    void                          *ConIn;
    EFI_HANDLE                     ConsoleOutHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut;
    EFI_HANDLE                     ConsoleErrorHandle;
    void                          *ConErr;
    EFI_RUNTIME_SERVICES          *RuntimeServices;
    EFI_BOOT_SERVICES             *BootServices;
    UINTN                          NumberOfTableEntries;
    struct { EFI_GUID VendorGuid; void *VendorTable; } *ConfigurationTable;
} EFI_SYSTEM_TABLE;
