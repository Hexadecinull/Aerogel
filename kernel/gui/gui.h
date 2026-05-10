#pragma once
#include <types.h>
#include <framebuffer.h>

#define GUI_TITLEBAR_H  20
#define GUI_BORDER      2
#define GUI_MAX_WINDOWS 8

typedef struct {
    int   x, y, w, h;
    char  title[64];
    bool  visible;
    u32   bg_color;
    void (*on_key)(char c);
} gui_window_t;

void gui_init(void);
void gui_draw_desktop(void);
gui_window_t *gui_create_window(const char *title, int x, int y, int w, int h);
void          gui_destroy_window(gui_window_t *win);
void          gui_draw_window(gui_window_t *win);
void          gui_draw_all(void);
void          gui_window_print(gui_window_t *win, const char *text, u32 fg);
void          gui_window_clear(gui_window_t *win);
void          gui_dispatch_key(char c);
