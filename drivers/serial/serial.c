#include "serial.h"
#include <io.h>

#define COM1 0x3F8

void serial_init(void) {
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x80);
    outb(COM1 + 0, 0x01);
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x03);
    outb(COM1 + 2, 0xC7);
    outb(COM1 + 4, 0x0B);
}

int serial_ready(void) {
    return inb(COM1 + 5) & 0x20;
}

void serial_putc(char c) {
    while (!serial_ready());
    if (c == '\n') {
        outb(COM1, '\r');
        while (!serial_ready());
    }
    outb(COM1, (u8)c);
}

void serial_puts(const char *s) {
    while (*s) serial_putc(*s++);
}
