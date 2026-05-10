#pragma once
#include <types.h>

typedef struct {
    u16 limit_low;
    u16 base_low;
    u8  base_mid;
    u8  access;
    u8  gran;
    u8  base_high;
} PACKED gdt_entry_t;

typedef struct {
    u16 limit;
    u64 base;
} PACKED gdt_ptr_t;

void gdt_init(void);
