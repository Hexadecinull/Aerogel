#include "pmm.h"
#include <string.h>
#include <kprintf.h>

#define MAX_PAGES   (512 * 1024)          /* covers 2 GiB */
#define BITMAP_BYTES (MAX_PAGES / 8)

static u8  bitmap[BITMAP_BYTES];
static u64 total_bytes = 0;
static u64 used_bytes  = 0;

static void set_page(uptr idx)   { bitmap[idx >> 3] |=  (u8)(1 << (idx & 7)); }
static void clear_page(uptr idx) { bitmap[idx >> 3] &= (u8)~(1 << (idx & 7)); }
static int  test_page(uptr idx)  { return (bitmap[idx >> 3] >> (idx & 7)) & 1; }

static uptr addr_to_page(u64 a) { return (uptr)(a >> PAGE_SHIFT); }

void pmm_init(e820_entry_t *map, u32 count) {
    memset(bitmap, 0xFF, sizeof(bitmap));

    for (u32 i = 0; i < count; i++) {
        if (map[i].type != E820_TYPE_USABLE) continue;

        u64 start = PAGE_ALIGN(map[i].base);
        u64 end   = (map[i].base + map[i].length) & ~(PAGE_SIZE - 1);
        if (end <= start) continue;

        total_bytes += end - start;

        uptr p     = addr_to_page(start);
        uptr p_end = addr_to_page(end);
        for (; p < p_end && p < MAX_PAGES; p++) {
            clear_page(p);
        }
    }

    pmm_mark_used(0x000000, 0x100000);

    extern u8 _bss_end[];
    uptr kernel_end = PAGE_ALIGN((uptr)_bss_end);
    pmm_mark_used(0x10000, kernel_end);

    uptr bm_start = (uptr)bitmap;
    uptr bm_end   = PAGE_ALIGN(bm_start + sizeof(bitmap));
    pmm_mark_used(bm_start, bm_end);

    used_bytes = 0;
    for (uptr i = 0; i < MAX_PAGES; i++)
        if (test_page(i)) used_bytes += PAGE_SIZE;

    kprintf("[PMM] Total: %u MiB  Free: %u MiB\n",
            (u32)(total_bytes   / (1024 * 1024)),
            (u32)(pmm_available() / (1024 * 1024)));
}

void pmm_mark_used(uptr start, uptr end) {
    uptr p     = addr_to_page(start);
    uptr p_end = addr_to_page(PAGE_ALIGN(end));
    for (; p < p_end && p < MAX_PAGES; p++) {
        if (!test_page(p)) { set_page(p); used_bytes += PAGE_SIZE; }
    }
}

uptr pmm_alloc(void) {
    for (uptr i = 1; i < MAX_PAGES; i++) {
        if (!test_page(i)) {
            set_page(i);
            used_bytes += PAGE_SIZE;
            return (uptr)(i << PAGE_SHIFT);
        }
    }
    return 0;
}

uptr pmm_alloc_n(u32 n) {
    if (!n) return 0;
    uptr run = 0, start = 0;
    for (uptr i = 1; i < MAX_PAGES; i++) {
        if (!test_page(i)) {
            if (!run) start = i;
            if (++run == n) {
                for (uptr j = start; j < start + n; j++) {
                    set_page(j);
                    used_bytes += PAGE_SIZE;
                }
                return (uptr)(start << PAGE_SHIFT);
            }
        } else {
            run = 0;
        }
    }
    return 0;
}

void pmm_free(uptr phys) {
    uptr idx = phys >> PAGE_SHIFT;
    if (idx >= MAX_PAGES || !test_page(idx)) return;
    clear_page(idx);
    used_bytes -= PAGE_SIZE;
}

void pmm_free_n(uptr phys, u32 n) {
    for (u32 i = 0; i < n; i++)
        pmm_free(phys + (uptr)(i << PAGE_SHIFT));
}

u64 pmm_total(void)     { return total_bytes; }
u64 pmm_used(void)      { return used_bytes; }
u64 pmm_available(void) { return total_bytes - used_bytes; }
