#pragma once
#include <types.h>

typedef struct {
    u32  addr;
    u32  pitch;
    u16  width;
    u16  height;
    u8   bpp;
    bool active;
} fb_info_t;

bool fb_init(void);
bool fb_active(void);

void fb_clear(u32 color);
void fb_pixel(int x, int y, u32 color);
void fb_rect(int x, int y, int w, int h, u32 color);
void fb_rect_outline(int x, int y, int w, int h, int t, u32 color);
void fb_line(int x0, int y0, int x1, int y1, u32 color);
void fb_char(int x, int y, char c, u32 fg, u32 bg);
void fb_string(int x, int y, const char *s, u32 fg, u32 bg);
void fb_hline(int x, int y, int len, u32 color);
void fb_vline(int x, int y, int len, u32 color);

u16 fb_width(void);
u16 fb_height(void);

#define FB_FONT_W 8
#define FB_FONT_H 16

#define RGB(r,g,b) ((u32)(r)<<16|(u32)(g)<<8|(u32)(b))
#define COLOR_BLACK   RGB(0x00,0x00,0x00)
#define COLOR_WHITE   RGB(0xFF,0xFF,0xFF)
#define COLOR_GRAY    RGB(0xC0,0xC0,0xC0)
#define COLOR_DGRAY   RGB(0x40,0x40,0x40)
#define COLOR_RED     RGB(0xCC,0x00,0x00)
#define COLOR_GREEN   RGB(0x00,0xCC,0x00)
#define COLOR_BLUE    RGB(0x00,0x00,0xCC)
#define COLOR_CYAN    RGB(0x00,0xCC,0xCC)
#define COLOR_YELLOW  RGB(0xCC,0xCC,0x00)
#define COLOR_ORANGE  RGB(0xFF,0x88,0x00)
#define COLOR_NAVY    RGB(0x00,0x00,0x80)
#define COLOR_LTBLUE  RGB(0x60,0x90,0xD0)
