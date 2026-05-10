#pragma once
#include <types.h>

typedef struct {
    u16 offset_low;
    u16 selector;
    u8  ist;
    u8  flags;
    u16 offset_mid;
    u32 offset_high;
    u32 zero;
} PACKED idt_entry_t;

typedef struct {
    u16 limit;
    u64 base;
} PACKED idt_ptr_t;

void idt_init(void);
void idt_set_gate(u8 num, u64 base, u16 sel, u8 flags);
