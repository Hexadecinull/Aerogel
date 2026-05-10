#include "gui.h"
#include <framebuffer.h>
#include <string.h>
#include <stdlib.h>

static gui_window_t windows[GUI_MAX_WINDOWS];
static int          nwindows = 0;

static int  cursor_x[GUI_MAX_WINDOWS];
static int  cursor_y[GUI_MAX_WINDOWS];

static u32 desktop_bg   = COLOR_NAVY;
static u32 taskbar_bg   = RGB(0x20, 0x40, 0x80);
static u32 title_active = COLOR_LTBLUE;
static u32 title_text   = COLOR_WHITE;
static u32 border_col   = RGB(0x90, 0xB0, 0xD0);
static u32 win_bg       = RGB(0xF0, 0xF0, 0xF0);

void gui_init(void) {
    memset(windows, 0, sizeof(windows));
    nwindows = 0;
    gui_draw_desktop();
}

void gui_draw_desktop(void) {
    int w = fb_width(), h = fb_height();
    fb_clear(desktop_bg);

    for (int y = 0; y < h; y++) {
        u8 shade = (u8)(0x00 + y * 0x30 / h);
        fb_hline(0, y, w, RGB(0x00, shade, 0x80 + shade));
    }

    int th = 28;
    fb_rect(0, h - th, w, th, taskbar_bg);
    fb_hline(0, h - th, w, COLOR_LTBLUE);

    fb_string(8, h - th + 6,
              "Aerogel OS v0.1.0", COLOR_WHITE, taskbar_bg);

    fb_string(w - 100, h - th + 6,
              "Phase 8", COLOR_CYAN, taskbar_bg);
}

gui_window_t *gui_create_window(const char *title, int x, int y, int w, int h) {
    if (nwindows >= GUI_MAX_WINDOWS) return NULL;
    gui_window_t *win = &windows[nwindows];
    int idx = nwindows++;
    memset(win, 0, sizeof(*win));
    win->x        = x;
    win->y        = y;
    win->w        = w;
    win->h        = h;
    win->visible  = true;
    win->bg_color = win_bg;
    strncpy(win->title, title, 63);
    cursor_x[idx] = 4;
    cursor_y[idx] = GUI_TITLEBAR_H + 4;
    gui_draw_window(win);
    return win;
}

void gui_draw_window(gui_window_t *win) {
    if (!win || !win->visible) return;

    fb_rect(win->x, win->y, win->w, win->h, win_bg);

    fb_rect(win->x, win->y, win->w, GUI_TITLEBAR_H, title_active);

    fb_string(win->x + 6, win->y + 3, win->title,
              title_text, title_active);

    int cx = win->x + win->w - 18;
    int cy = win->y + 3;
    fb_rect(cx, cy, 14, 14, COLOR_RED);
    fb_string(cx + 4, cy + 1, "x", COLOR_WHITE, COLOR_RED);

    fb_rect_outline(win->x, win->y, win->w, win->h, GUI_BORDER, border_col);
}

void gui_draw_all(void) {
    gui_draw_desktop();
    for (int i = 0; i < nwindows; i++)
        gui_draw_window(&windows[i]);
}

void gui_destroy_window(gui_window_t *win) {
    if (!win) return;
    win->visible = false;
    gui_draw_desktop();
    for (int i = 0; i < nwindows; i++)
        if (&windows[i] != win) gui_draw_window(&windows[i]);
}

void gui_window_print(gui_window_t *win, const char *text, u32 fg) {
    int idx = (int)(win - windows);
    int *cx = &cursor_x[idx];
    int *cy = &cursor_y[idx];
    int  client_right = win->x + win->w - GUI_BORDER - 2;
    int  client_bot   = win->y + win->h - GUI_BORDER - 2;

    while (*text) {
        char c = *text++;
        if (c == '\n') {
            *cx  = 4;
            *cy += FB_FONT_H;
        } else {
            if (win->x + *cx + FB_FONT_W > client_right) {
                *cx  = 4;
                *cy += FB_FONT_H;
            }
            if (win->y + *cy + FB_FONT_H > client_bot) {
                *cy = GUI_TITLEBAR_H + 4;
                fb_rect(win->x + 2, win->y + GUI_TITLEBAR_H,
                        win->w - 4, win->h - GUI_TITLEBAR_H - 2,
                        win->bg_color);
            }
            fb_char(win->x + *cx, win->y + *cy, c, fg, win->bg_color);
            *cx += FB_FONT_W;
        }
    }
}

void gui_window_clear(gui_window_t *win) {
    int idx = (int)(win - windows);
    cursor_x[idx] = 4;
    cursor_y[idx] = GUI_TITLEBAR_H + 4;
    fb_rect(win->x + GUI_BORDER, win->y + GUI_TITLEBAR_H,
            win->w - GUI_BORDER*2,
            win->h - GUI_TITLEBAR_H - GUI_BORDER,
            win->bg_color);
}

void gui_dispatch_key(char c) {
    for (int i = 0; i < nwindows; i++) {
        if (windows[i].visible && windows[i].on_key)
            windows[i].on_key(c);
    }
}
