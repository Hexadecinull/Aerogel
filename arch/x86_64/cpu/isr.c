#include "isr.h"
#include "idt.h"
#include <io.h>
#include <panic.h>
#include <string.h>

#define PIC1_CMD  0x20
#define PIC1_DATA 0x21
#define PIC2_CMD  0xA0
#define PIC2_DATA 0xA1

static isr_handler_t handlers[256];

static void pic_remap(void) {
    outb(PIC1_CMD,  0x11); io_wait();
    outb(PIC2_CMD,  0x11); io_wait();
    outb(PIC1_DATA, 0x20); io_wait();
    outb(PIC2_DATA, 0x28); io_wait();
    outb(PIC1_DATA, 0x04); io_wait();
    outb(PIC2_DATA, 0x02); io_wait();
    outb(PIC1_DATA, 0x01); io_wait();
    outb(PIC2_DATA, 0x01); io_wait();
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
}

void pic_send_eoi(u8 irq) {
    if (irq >= 8) outb(PIC2_CMD, 0x20);
    outb(PIC1_CMD, 0x20);
}

void pic_mask_irq(u8 irq) {
    u16 port = (irq < 8) ? PIC1_DATA : PIC2_DATA;
    u8  bit  = (irq < 8) ? irq : irq - 8;
    outb(port, inb(port) | (u8)(1 << bit));
}

void pic_unmask_irq(u8 irq) {
    u16 port = (irq < 8) ? PIC1_DATA : PIC2_DATA;
    u8  bit  = (irq < 8) ? irq : irq - 8;
    outb(port, inb(port) & (u8)~(1 << bit));
}

void isr_register(u8 num, isr_handler_t handler) { handlers[num] = handler; }

void isr_init(void) {
    memset(handlers, 0, sizeof(handlers));
    pic_remap();
    idt_init();
}

static const char *exc_names[32] = {
    "Divide Error","Debug","NMI","Breakpoint","Overflow",
    "BOUND","Invalid Opcode","Device Not Available","Double Fault",
    "Coprocessor Segment Overrun","Invalid TSS","Segment Not Present",
    "Stack Fault","General Protection","Page Fault","Reserved",
    "x87 FP","Alignment Check","Machine Check","SIMD FP",
    "Virtualization","Control Protection",
    "Rsv","Rsv","Rsv","Rsv","Rsv","Rsv","Rsv","Rsv",
    "Security","Reserved",
};

void isr_dispatch(isr_frame_t *frame) {
    u64 num = frame->int_no;

    if (handlers[num]) {
        handlers[num](frame);
    } else if (num < 32) {
        kpanic("Exception #%u: %s  err=%x  rip=%x",
               (u32)num, exc_names[num],
               (u32)frame->err_code, (u64)frame->rip);
    }

    if (num >= 32 && num < 48)
        pic_send_eoi((u8)(num - 32));
}
