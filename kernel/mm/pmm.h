#pragma once
#include <types.h>

#define PAGE_SIZE      4096UL
#define PAGE_SHIFT     12
#define PAGE_ALIGN(a)  ALIGN_UP((a), PAGE_SIZE)

#define E820_TYPE_USABLE   1
#define E820_TYPE_RESERVED 2
#define E820_TYPE_ACPI     3
#define E820_TYPE_NVS      4
#define E820_TYPE_BAD      5

typedef struct {
    u64 base;
    u64 length;
    u32 type;
    u32 acpi;
} PACKED e820_entry_t;

void  pmm_init(e820_entry_t *map, u32 count);
uptr  pmm_alloc(void);
uptr  pmm_alloc_n(u32 pages);
void  pmm_free(uptr phys);
void  pmm_free_n(uptr phys, u32 pages);
u64   pmm_total(void);
u64   pmm_used(void);
u64   pmm_available(void);
void  pmm_mark_used(uptr start, uptr end);
