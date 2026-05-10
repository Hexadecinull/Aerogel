#pragma once
#include <types.h>

typedef struct {
    u64 r15, r14, r13, r12, r11, r10, r9, r8;
    u64 rdi, rsi, rbp, rdx, rcx, rbx, rax;
    u64 int_no, err_code;
    u64 rip, cs, rflags, rsp, ss;
} PACKED isr_frame_t;

typedef void (*isr_handler_t)(isr_frame_t *frame);

void isr_init(void);
void isr_register(u8 num, isr_handler_t handler);
void isr_dispatch(isr_frame_t *frame);
void pic_unmask_irq(u8 irq);
void pic_mask_irq(u8 irq);
void pic_send_eoi(u8 irq);
