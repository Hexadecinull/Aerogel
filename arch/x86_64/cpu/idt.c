#include "idt.h"
#include "isr.h"

static idt_entry_t idt[256];
static idt_ptr_t   idt_ptr;

#define DECL_ISR(n) extern void isr_##n(void);
DECL_ISR(0)  DECL_ISR(1)  DECL_ISR(2)  DECL_ISR(3)  DECL_ISR(4)
DECL_ISR(5)  DECL_ISR(6)  DECL_ISR(7)  DECL_ISR(8)  DECL_ISR(9)
DECL_ISR(10) DECL_ISR(11) DECL_ISR(12) DECL_ISR(13) DECL_ISR(14)
DECL_ISR(15) DECL_ISR(16) DECL_ISR(17) DECL_ISR(18) DECL_ISR(19)
DECL_ISR(20) DECL_ISR(21) DECL_ISR(22) DECL_ISR(23) DECL_ISR(24)
DECL_ISR(25) DECL_ISR(26) DECL_ISR(27) DECL_ISR(28) DECL_ISR(29)
DECL_ISR(30) DECL_ISR(31) DECL_ISR(32) DECL_ISR(33) DECL_ISR(34)
DECL_ISR(35) DECL_ISR(36) DECL_ISR(37) DECL_ISR(38) DECL_ISR(39)
DECL_ISR(40) DECL_ISR(41) DECL_ISR(42) DECL_ISR(43) DECL_ISR(44)
DECL_ISR(45) DECL_ISR(46) DECL_ISR(47)

void idt_set_gate(u8 num, u64 base, u16 sel, u8 flags) {
    idt[num].offset_low  = (u16)(base & 0xFFFF);
    idt[num].offset_mid  = (u16)((base >> 16) & 0xFFFF);
    idt[num].offset_high = (u32)((base >> 32) & 0xFFFFFFFF);
    idt[num].selector    = sel;
    idt[num].ist         = 0;
    idt[num].flags       = flags;
    idt[num].zero        = 0;
}

void idt_init(void) {
    idt_ptr.limit = (u16)(sizeof(idt) - 1);
    idt_ptr.base  = (u64)&idt;

    static void (*stubs[48])(void) = {
        isr_0,  isr_1,  isr_2,  isr_3,  isr_4,  isr_5,  isr_6,  isr_7,
        isr_8,  isr_9,  isr_10, isr_11, isr_12, isr_13, isr_14, isr_15,
        isr_16, isr_17, isr_18, isr_19, isr_20, isr_21, isr_22, isr_23,
        isr_24, isr_25, isr_26, isr_27, isr_28, isr_29, isr_30, isr_31,
        isr_32, isr_33, isr_34, isr_35, isr_36, isr_37, isr_38, isr_39,
        isr_40, isr_41, isr_42, isr_43, isr_44, isr_45, isr_46, isr_47,
    };

    for (int i = 0; i < 48; i++)
        idt_set_gate((u8)i, (u64)stubs[i], 0x08, 0x8E);

    __asm__ volatile("lidt %0" : : "m"(idt_ptr));
}
