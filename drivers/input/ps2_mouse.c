#include "ps2_mouse.h"
#include <io.h>
#include <isr.h>

#define PS2_DATA   0x60
#define PS2_STATUS 0x64
#define PS2_CMD    0x64

static mouse_state_t state;
static u8  pkt[3];
static int pkt_idx = 0;

static void mouse_write(u8 data) {
    while (inb(PS2_STATUS) & 0x02);
    outb(PS2_CMD, 0xD4);
    while (inb(PS2_STATUS) & 0x02);
    outb(PS2_DATA, data);
}

static u8 mouse_read(void) {
    while (!(inb(PS2_STATUS) & 0x01));
    return inb(PS2_DATA);
}

static void mouse_handler(isr_frame_t *frame) {
    (void)frame;
    u8 byte = inb(PS2_DATA);

    if (pkt_idx == 0 && !(byte & 0x08)) return;

    pkt[pkt_idx++] = byte;
    if (pkt_idx < 3) return;
    pkt_idx = 0;

    state.left   = !!(pkt[0] & 0x01);
    state.right  = !!(pkt[0] & 0x02);
    state.middle = !!(pkt[0] & 0x04);

    i32 dx = (i32)pkt[1] - ((pkt[0] & 0x10) ? 256 : 0);
    i32 dy = (i32)pkt[2] - ((pkt[0] & 0x20) ? 256 : 0);
    state.x += dx;
    state.y -= dy;
}

void mouse_init(void) {
    while (inb(PS2_STATUS) & 0x02);
    outb(PS2_CMD, 0xA8);

    while (inb(PS2_STATUS) & 0x02);
    outb(PS2_CMD, 0x20);
    while (!(inb(PS2_STATUS) & 0x01));
    u8 cfg = inb(PS2_DATA);
    cfg |= 0x02;
    while (inb(PS2_STATUS) & 0x02);
    outb(PS2_CMD, 0x60);
    while (inb(PS2_STATUS) & 0x02);
    outb(PS2_DATA, cfg);

    mouse_write(0xFF); mouse_read();
    mouse_write(0xF6); mouse_read();
    mouse_write(0xF4); mouse_read();

    isr_register(44, mouse_handler);
    pic_unmask_irq(12);
}

const mouse_state_t *mouse_get(void) { return &state; }
