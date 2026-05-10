#pragma once
#include <types.h>

typedef struct {
    u32 ds;
    u32 edi, esi, ebp, esp_dummy, ebx, edx, ecx, eax;
    u32 int_no, err_code;
    u32 eip, cs, eflags, esp, ss;
} PACKED isr_frame_t;

typedef void (*isr_handler_t)(isr_frame_t *frame);

void isr_init(void);
void isr_register(u8 num, isr_handler_t handler);
void isr_dispatch(isr_frame_t *frame);
void pic_unmask_irq(u8 irq);
void pic_mask_irq(u8 irq);
void pic_send_eoi(u8 irq);
