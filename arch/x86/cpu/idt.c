#include "idt.h"
#include "isr.h"

static idt_entry_t idt[256];
static idt_ptr_t   idt_ptr;

extern void isr_0(void);  extern void isr_1(void);  extern void isr_2(void);
extern void isr_3(void);  extern void isr_4(void);  extern void isr_5(void);
extern void isr_6(void);  extern void isr_7(void);  extern void isr_8(void);
extern void isr_9(void);  extern void isr_10(void); extern void isr_11(void);
extern void isr_12(void); extern void isr_13(void); extern void isr_14(void);
extern void isr_15(void); extern void isr_16(void); extern void isr_17(void);
extern void isr_18(void); extern void isr_19(void); extern void isr_20(void);
extern void isr_21(void); extern void isr_22(void); extern void isr_23(void);
extern void isr_24(void); extern void isr_25(void); extern void isr_26(void);
extern void isr_27(void); extern void isr_28(void); extern void isr_29(void);
extern void isr_30(void); extern void isr_31(void);
extern void isr_32(void); extern void isr_33(void); extern void isr_34(void);
extern void isr_35(void); extern void isr_36(void); extern void isr_37(void);
extern void isr_38(void); extern void isr_39(void); extern void isr_40(void);
extern void isr_41(void); extern void isr_42(void); extern void isr_43(void);
extern void isr_44(void); extern void isr_45(void); extern void isr_46(void);
extern void isr_47(void);

void idt_set_gate(u8 num, u32 base, u16 sel, u8 flags) {
    idt[num].offset_low  = base & 0xFFFF;
    idt[num].offset_high = (base >> 16) & 0xFFFF;
    idt[num].selector    = sel;
    idt[num].zero        = 0;
    idt[num].flags       = flags;
}

void idt_init(void) {
    idt_ptr.limit = (u16)(sizeof(idt) - 1);
    idt_ptr.base  = (u32)&idt;

    static void (*stubs[48])(void) = {
        isr_0,  isr_1,  isr_2,  isr_3,  isr_4,  isr_5,  isr_6,  isr_7,
        isr_8,  isr_9,  isr_10, isr_11, isr_12, isr_13, isr_14, isr_15,
        isr_16, isr_17, isr_18, isr_19, isr_20, isr_21, isr_22, isr_23,
        isr_24, isr_25, isr_26, isr_27, isr_28, isr_29, isr_30, isr_31,
        isr_32, isr_33, isr_34, isr_35, isr_36, isr_37, isr_38, isr_39,
        isr_40, isr_41, isr_42, isr_43, isr_44, isr_45, isr_46, isr_47,
    };

    for (int i = 0; i < 48; i++)
        idt_set_gate((u8)i, (u32)stubs[i], 0x08, 0x8E);

    __asm__ volatile("lidt %0" : : "m"(idt_ptr));
}
