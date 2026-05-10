
#include "efi.h"

#define KERNEL_PHYS    0x200000ULL
#define KERNEL_SECTORS 128
#define E820_ADDR      0x500ULL
#define E820_CNT_ADDR  0x4FCULL

#define PAGE_SIZE 4096ULL
#define PT_PRESENT  (1ULL << 0)
#define PT_WRITABLE (1ULL << 1)
#define PT_HUGE     (1ULL << 7)

typedef struct {
    UINT64 base;
    UINT64 length;
    UINT32 type;
    UINT32 acpi;
} __attribute__((packed)) e820_entry_t;

static UINT64 pml4[512] __attribute__((aligned(4096)));
static UINT64 pdpt[512] __attribute__((aligned(4096)));
static UINT64 pd[4][512] __attribute__((aligned(4096)));

static void build_page_tables(void) {
    for (int i = 0; i < 512; i++) pml4[i] = pdpt[i] = 0;
    for (int g = 0; g < 4; g++)
        for (int i = 0; i < 512; i++) pd[g][i] = 0;

    pml4[0] = (UINT64)pdpt | PT_PRESENT | PT_WRITABLE;

    for (int g = 0; g < 4; g++) {
        pdpt[g] = (UINT64)pd[g] | PT_PRESENT | PT_WRITABLE;
        for (int i = 0; i < 512; i++)
            pd[g][i] = (UINT64)((g * 512ULL + i) * 0x200000ULL)
                       | PT_PRESENT | PT_WRITABLE | PT_HUGE;
    }
}

static void build_e820(EFI_SYSTEM_TABLE *st,
                        EFI_MEMORY_DESCRIPTOR *efi_map,
                        UINTN map_size, UINTN desc_size) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
    e820_entry_t *e820 = (e820_entry_t *)E820_ADDR;
    UINT32 count = 0;

    UINT8 *p = (UINT8 *)efi_map;
    UINT8 *end = p + map_size;
    while (p < end && count < 128) {
        EFI_MEMORY_DESCRIPTOR *d = (EFI_MEMORY_DESCRIPTOR *)p;
        UINT32 type = (d->Type == EfiConventionalMemory ||
                       d->Type == EfiLoaderCode ||
                       d->Type == EfiLoaderData) ? 1 : 2;
        e820[count].base   = d->PhysicalStart;
        e820[count].length = d->NumberOfPages * PAGE_SIZE;
        e820[count].type   = type;
        e820[count].acpi   = 0;
        count++;
        p += desc_size;
    }
    volatile UINT32 *cnt_ptr = (volatile UINT32 *)E820_CNT_ADDR;
    *cnt_ptr = count;
#pragma GCC diagnostic pop
}

__attribute__((ms_abi))
EFI_STATUS efi_main(EFI_HANDLE img, EFI_SYSTEM_TABLE *st) {
    EFI_BOOT_SERVICES *bs = st->BootServices;

    st->ConOut->OutputString(st->ConOut,
        (CHAR16[]){
            'A','e','r','o','g','e','l',' ','U','E','F','I','\r','\n',0
        });

    /* --- Load kernel from filesystem --- */
    EFI_GUID lip_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_LOADED_IMAGE_PROTOCOL *lip = 0;
    bs->HandleProtocol(img, &lip_guid, (void**)&lip);

    EFI_GUID sfs_guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *sfs = 0;
    bs->HandleProtocol(lip->DeviceHandle, &sfs_guid, (void**)&sfs);

    EFI_FILE_PROTOCOL *root = 0;
    sfs->OpenVolume((void*)sfs, &root);

    EFI_FILE_PROTOCOL *kf = 0;
    CHAR16 kpath[] = {'\\','a','e','r','o','g','e','l','\\',
                      'k','e','r','n','e','l','.','b','i','n',0};
    EFI_STATUS s = root->Open(root, &kf, kpath, EFI_FILE_MODE_READ, 0);
    if (s != EFI_SUCCESS) {
        st->ConOut->OutputString(st->ConOut,
            (CHAR16[]){'K','e','r','n','e','l',' ','n','o','t',' ',
                       'f','o','u','n','d','\r','\n',0});
        for(;;) __asm__ volatile("hlt");
    }

    EFI_PHYSICAL_ADDRESS kaddr = KERNEL_PHYS;
    bs->AllocatePages(AllocateAddress, EfiLoaderData,
                      KERNEL_SECTORS, &kaddr);

    UINTN ksize = KERNEL_SECTORS * PAGE_SIZE;
    kf->Read(kf, &ksize, (void *)kaddr);
    kf->Close(kf);
    root->Close(root);

    /* --- Get memory map --- */
    UINTN map_size = 0, map_key = 0, desc_size = 0;
    UINT32 desc_ver = 0;
    bs->GetMemoryMap(&map_size, 0, &map_key, &desc_size, &desc_ver);
    map_size += desc_size * 32;

    EFI_PHYSICAL_ADDRESS map_buf = 0;
    bs->AllocatePages(AllocateAnyPages, EfiLoaderData,
                      (map_size + PAGE_SIZE - 1) / PAGE_SIZE, &map_buf);
    EFI_MEMORY_DESCRIPTOR *efi_map = (EFI_MEMORY_DESCRIPTOR *)map_buf;
    bs->GetMemoryMap(&map_size, efi_map, &map_key, &desc_size, &desc_ver);

    /* --- Build own page tables (identity map 0–8 GiB, 2 MiB pages) --- */
    build_page_tables();

    /* --- Exit boot services --- */
    /* Retry once: GetMemoryMap may invalidate key on AllocatePages */
    s = bs->ExitBootServices(img, map_key);
    if (s != EFI_SUCCESS) {
        map_size += desc_size * 8;
        bs->GetMemoryMap(&map_size, efi_map, &map_key, &desc_size, &desc_ver);
        bs->ExitBootServices(img, map_key);
    }

    /* --- Now in firmware-free environment --- */
    build_e820(st, efi_map, map_size, desc_size);

    /* Switch to our page tables */
    __asm__ volatile("mov %0, %%cr3" : : "r"((UINT64)pml4) : "memory");

    /* Jump to kernel */
    typedef void (*kernel_entry_t)(void);
    ((kernel_entry_t)kaddr)();

    for (;;) __asm__ volatile("hlt");
    return EFI_SUCCESS;
}
