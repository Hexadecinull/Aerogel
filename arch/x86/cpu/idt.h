#pragma once
#include <types.h>

typedef struct {
    u16 offset_low;
    u16 selector;
    u8  zero;
    u8  flags;
    u16 offset_high;
} PACKED idt_entry_t;

typedef struct {
    u16 limit;
    u32 base;
} PACKED idt_ptr_t;

void idt_init(void);
void idt_set_gate(u8 num, u32 base, u16 sel, u8 flags);
