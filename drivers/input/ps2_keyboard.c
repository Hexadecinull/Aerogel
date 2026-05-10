#include "ps2_keyboard.h"
#include <io.h>
#include <isr.h>
#include <string.h>

#define PS2_DATA   0x60
#define PS2_STATUS 0x64
#define PS2_CMD    0x64

#define KEY_BUF_SIZE 128

static const char sc_ascii[128] = {
    0,    0x1B, '1',  '2',  '3',  '4',  '5',  '6',
    '7',  '8',  '9',  '0',  '-',  '=',  0x08, 0x09,
    'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',
    'o',  'p',  '[',  ']',  0x0A, 0,    'a',  's',
    'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',
    0x27, '`',  0,    0x5C, 'z',  'x',  'c',  'v',
    'b',  'n',  'm',  ',',  '.',  '/',  0,    '*',
    0,    ' ',  0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    '7',  '8',  '9',  '-',  '4',  '5',  '6',
    '+',  '1',  '2',  '3',  '0',  '.',  0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
};

static const char sc_shift[128] = {
    0,    0x1B, '!',  '@',  '#',  '$',  '%',  '^',
    '&',  '*',  '(',  ')',  '_',  '+',  0x08, 0x09,
    'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',
    'O',  'P',  '{',  '}',  0x0A, 0,    'A',  'S',
    'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',
    0x22, '~',  0,    '|',  'Z',  'X',  'C',  'V',
    'B',  'N',  'M',  '<',  '>',  '?',  0,    '*',
    0,    ' ',  0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    '7',  '8',  '9',  '-',  '4',  '5',  '6',
    '+',  '1',  '2',  '3',  '0',  '.',  0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
};

static key_event_t kbuf[KEY_BUF_SIZE];
static volatile u32 khead = 0, ktail = 0;
static u8   modifiers = 0;
static bool extended  = false;

static bool kbuf_push(key_event_t *e) {
    u32 next = (khead + 1) % KEY_BUF_SIZE;
    if (next == ktail) return false;
    kbuf[khead] = *e;
    khead = next;
    return true;
}

bool keyboard_poll(key_event_t *evt) {
    if (khead == ktail) return false;
    *evt  = kbuf[ktail];
    ktail = (ktail + 1) % KEY_BUF_SIZE;
    return true;
}

u8 keyboard_modifiers(void) { return modifiers; }

static void kb_handler(isr_frame_t *frame) {
    (void)frame;
    u8 sc = inb(PS2_DATA);

    if (sc == 0xE0) { extended = true; return; }

    bool pressed = !(sc & 0x80);
    u8   code    = sc & 0x7F;
    extended = false;

    switch (code) {
    case 0x2A: case 0x36:
        if (pressed) modifiers |=  KEY_MOD_SHIFT;
        else         modifiers &= (u8)~KEY_MOD_SHIFT;
        return;
    case 0x1D:
        if (pressed) modifiers |=  KEY_MOD_CTRL;
        else         modifiers &= (u8)~KEY_MOD_CTRL;
        return;
    case 0x38:
        if (pressed) modifiers |=  KEY_MOD_ALT;
        else         modifiers &= (u8)~KEY_MOD_ALT;
        return;
    case 0x3A:
        if (pressed) modifiers ^= KEY_MOD_CAPS;
        return;
    }

    if (!pressed) return;

    char c = 0;
    if (code < 128) {
        bool shift = !!(modifiers & KEY_MOD_SHIFT);
        bool caps  = !!(modifiers & KEY_MOD_CAPS);
        c = shift ? sc_shift[code] : sc_ascii[code];
        if (caps && c >= 'a' && c <= 'z') c = (char)(c - 32);
        if (caps && c >= 'A' && c <= 'Z') c = (char)(c + 32);
    }

    key_event_t e = {
        .scancode  = code,
        .ascii     = c,
        .modifiers = modifiers,
        .pressed   = true
    };
    kbuf_push(&e);
}

static void ps2_flush(void) {
    while (inb(PS2_STATUS) & 0x01) inb(PS2_DATA);
}

static void ps2_send_cmd(u8 cmd) {
    while (inb(PS2_STATUS) & 0x02);
    outb(PS2_CMD, cmd);
}

static void ps2_send_data(u8 data) {
    while (inb(PS2_STATUS) & 0x02);
    outb(PS2_DATA, data);
}

void keyboard_init(void) {
    ps2_send_cmd(0xAD);
    ps2_send_cmd(0xA7);
    ps2_flush();

    ps2_send_cmd(0x20);
    while (!(inb(PS2_STATUS) & 0x01));
    u8 cfg = inb(PS2_DATA);
    cfg |=  0x01;
    cfg &= (u8)~0x10;
    ps2_send_cmd(0x60);
    ps2_send_data(cfg);

    ps2_send_cmd(0xAA);
    while (!(inb(PS2_STATUS) & 0x01));
    inb(PS2_DATA);

    ps2_send_cmd(0xAB);
    while (!(inb(PS2_STATUS) & 0x01));
    inb(PS2_DATA);

    ps2_send_cmd(0xAE);

    ps2_send_data(0xFF);
    while (!(inb(PS2_STATUS) & 0x01));
    inb(PS2_DATA);

    ps2_send_data(0xF0);
    ps2_send_data(0x01);
    while (!(inb(PS2_STATUS) & 0x01));
    inb(PS2_DATA);

    ps2_flush();
    isr_register(33, kb_handler);
    pic_unmask_irq(1);
}

char keyboard_getc(void) {
    key_event_t e;
    for (;;) {
        if (keyboard_poll(&e) && e.ascii) return e.ascii;
        __asm__ volatile("hlt");
    }
}
