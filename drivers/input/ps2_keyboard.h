#pragma once
#include <types.h>

#define KEY_MOD_SHIFT 0x01
#define KEY_MOD_CTRL  0x02
#define KEY_MOD_ALT   0x04
#define KEY_MOD_CAPS  0x08

typedef struct {
    u8   scancode;
    char ascii;
    u8   modifiers;
    bool pressed;
} key_event_t;

void  keyboard_init(void);
bool  keyboard_poll(key_event_t *evt);
char  keyboard_getc(void);
u8    keyboard_modifiers(void);
