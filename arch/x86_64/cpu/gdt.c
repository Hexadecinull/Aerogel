#include "gdt.h"

static gdt_entry_t gdt[3];
static gdt_ptr_t   gdt_ptr;

extern void gdt_flush(gdt_ptr_t *ptr);

static void gdt_set(int i, u32 base, u32 limit, u8 access, u8 gran) {
    gdt[i].limit_low = limit & 0xFFFF;
    gdt[i].base_low  = base  & 0xFFFF;
    gdt[i].base_mid  = (base >> 16) & 0xFF;
    gdt[i].access    = access;
    gdt[i].gran      = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt[i].base_high = (base >> 24) & 0xFF;
}

void gdt_init(void) {
    gdt_ptr.limit = (u16)(sizeof(gdt) - 1);
    gdt_ptr.base  = (u64)&gdt;

    gdt_set(0, 0,          0,          0x00, 0x00);
    gdt_set(1, 0, 0xFFFFFFFF, 0x9A, 0xAF); /* 64-bit code: L=1, D=0 */
    gdt_set(2, 0, 0xFFFFFFFF, 0x92, 0xCF); /* data */

    gdt_flush(&gdt_ptr);
}
