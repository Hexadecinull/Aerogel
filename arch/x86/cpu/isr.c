#include "isr.h"
#include "idt.h"
#include <io.h>
#include <panic.h>
#include <string.h>

#define PIC1_CMD  0x20
#define PIC1_DATA 0x21
#define PIC2_CMD  0xA0
#define PIC2_DATA 0xA1
#define PIC_EOI   0x20

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
    if (irq >= 8) outb(PIC2_CMD, PIC_EOI);
    outb(PIC1_CMD, PIC_EOI);
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

void isr_register(u8 num, isr_handler_t handler) {
    handlers[num] = handler;
}

void isr_init(void) {
    memset(handlers, 0, sizeof(handlers));
    pic_remap();
    idt_init();
}

static const char *exception_names[32] = {
    "Divide Error",          "Debug",
    "NMI",                   "Breakpoint",
    "Overflow",              "BOUND Range Exceeded",
    "Invalid Opcode",        "Device Not Available",
    "Double Fault",          "Coprocessor Segment Overrun",
    "Invalid TSS",           "Segment Not Present",
    "Stack-Segment Fault",   "General Protection Fault",
    "Page Fault",            "Reserved",
    "x87 FP Exception",      "Alignment Check",
    "Machine Check",         "SIMD FP Exception",
    "Virtualization",        "Control Protection",
    "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved",
    "Security Exception",    "Reserved",
};

void isr_dispatch(isr_frame_t *frame) {
    u32 num = frame->int_no;

    if (handlers[num]) {
        handlers[num](frame);
    } else if (num < 32) {
        kpanic("Exception #%u: %s  err=%x  eip=%x  cs=%x",
               num,
               num < 32 ? exception_names[num] : "Unknown",
               frame->err_code, frame->eip, frame->cs);
    }

    if (num >= 32 && num < 48)
        pic_send_eoi((u8)(num - 32));
}
