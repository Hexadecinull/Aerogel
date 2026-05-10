#include "vga_text.h"
#include <io.h>
#include <string.h>

#define CRTC_ADDR 0x3D4
#define CRTC_DATA 0x3D5

static int  vga_row  = 0;
static int  vga_col  = 0;
static u8   vga_attr = 0x0F;

static u8 make_attr(vga_color_t fg, vga_color_t bg) {
    return (u8)((bg << 4) | (fg & 0x0F));
}

static void update_cursor(void) {
    u16 pos = (u16)(vga_row * VGA_COLS + vga_col);
    outb(CRTC_ADDR, 0x0E); outb(CRTC_DATA, (u8)(pos >> 8));
    outb(CRTC_ADDR, 0x0F); outb(CRTC_DATA, (u8)(pos & 0xFF));
}

static void scroll(void) {
    u16 blank = (u16)(' ') | ((u16)vga_attr << 8);
    for (int r = 1; r < VGA_ROWS; r++)
        for (int c = 0; c < VGA_COLS; c++)
            VGA_BASE[(r-1) * VGA_COLS + c] = VGA_BASE[r * VGA_COLS + c];
    for (int c = 0; c < VGA_COLS; c++)
        VGA_BASE[(VGA_ROWS-1) * VGA_COLS + c] = blank;
}

void vga_init(void) {
    vga_attr = make_attr(VGA_LIGHT_GRAY, VGA_BLACK);
    vga_clear();
    outb(CRTC_ADDR, 0x0A); outb(CRTC_DATA, 0x0E);
    outb(CRTC_ADDR, 0x0B); outb(CRTC_DATA, 0x0F);
    update_cursor();
}

void vga_clear(void) {
    u16 blank = (u16)(' ') | ((u16)vga_attr << 8);
    for (int i = 0; i < VGA_COLS * VGA_ROWS; i++)
        VGA_BASE[i] = blank;
    vga_row = vga_col = 0;
    update_cursor();
}

void vga_set_color(vga_color_t fg, vga_color_t bg) {
    vga_attr = make_attr(fg, bg);
}

void vga_get_pos(int *row, int *col) { *row = vga_row; *col = vga_col; }

void vga_set_pos(int row, int col) {
    vga_row = row; vga_col = col;
    update_cursor();
}

void vga_write_at(int row, int col, char c, u8 attr) {
    VGA_BASE[row * VGA_COLS + col] = (u16)(u8)c | ((u16)attr << 8);
}

void vga_puts_at(int row, int col, const char *s, u8 attr) {
    while (*s) { vga_write_at(row, col++, *s++, attr); }
}

void vga_putc(char c) {
    if (c == '\n') {
        vga_col = 0; vga_row++;
    } else if (c == '\r') {
        vga_col = 0;
    } else if (c == '\b') {
        if (vga_col > 0) {
            vga_col--;
            VGA_BASE[vga_row * VGA_COLS + vga_col] = (u16)' ' | ((u16)vga_attr << 8);
        }
    } else if (c == '\t') {
        vga_col = (vga_col + 8) & ~7;
        if (vga_col >= VGA_COLS) { vga_col = 0; vga_row++; }
    } else {
        VGA_BASE[vga_row * VGA_COLS + vga_col] = (u16)(u8)c | ((u16)vga_attr << 8);
        if (++vga_col >= VGA_COLS) { vga_col = 0; vga_row++; }
    }
    if (vga_row >= VGA_ROWS) { scroll(); vga_row = VGA_ROWS - 1; }
    update_cursor();
}

void vga_puts(const char *s) { while (*s) vga_putc(*s++); }
