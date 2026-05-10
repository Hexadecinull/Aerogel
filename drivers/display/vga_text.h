#pragma once
#include <types.h>

#define VGA_COLS 80
#define VGA_ROWS 25
#define VGA_BASE ((volatile u16 *)0xB8000)

typedef enum {
    VGA_BLACK         = 0,  VGA_BLUE          = 1,
    VGA_GREEN         = 2,  VGA_CYAN          = 3,
    VGA_RED           = 4,  VGA_MAGENTA       = 5,
    VGA_BROWN         = 6,  VGA_LIGHT_GRAY    = 7,
    VGA_DARK_GRAY     = 8,  VGA_LIGHT_BLUE    = 9,
    VGA_LIGHT_GREEN   = 10, VGA_LIGHT_CYAN    = 11,
    VGA_LIGHT_RED     = 12, VGA_LIGHT_MAGENTA = 13,
    VGA_YELLOW        = 14, VGA_WHITE         = 15,
} vga_color_t;

void vga_init(void);
void vga_clear(void);
void vga_putc(char c);
void vga_puts(const char *s);
void vga_set_color(vga_color_t fg, vga_color_t bg);
void vga_get_pos(int *row, int *col);
void vga_set_pos(int row, int col);
void vga_write_at(int row, int col, char c, u8 attr);
void vga_puts_at(int row, int col, const char *s, u8 attr);
