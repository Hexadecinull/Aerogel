#include "setup.h"
#include <kprintf.h>
#include <vga_text.h>
#include <ps2_keyboard.h>
#include <vfs.h>
#include <tmpfs.h>
#include <string.h>

static const char *LOGO[] = {
    " ___  ___  ___  ___  ___  ___  _    ",
    "| . || __||  _|| . || __|| __|| |   ",
    "|   || _| | |_ | | || _| | _| | |__",
    "|_|_||___||___||___||___||___||____|",
    "          Aerogel  OS  v0.1.0       ",
    NULL
};

static char hostname[64];

static char readln_buf[64];
static int  readln_len;

static void readln(void) {
    readln_len = 0;
    memset(readln_buf, 0, sizeof(readln_buf));
    for (;;) {
        key_event_t e;
        if (!keyboard_poll(&e)) { __asm__ volatile("hlt"); continue; }
        if (!e.pressed) continue;
        if (e.ascii == '\n' || e.ascii == '\r') {
            vga_putc('\n');
            readln_buf[readln_len] = 0;
            return;
        }
        if (e.ascii == '\b' && readln_len > 0) {
            readln_len--;
            vga_putc('\b');
            continue;
        }
        if (e.ascii && readln_len < 63) {
            readln_buf[readln_len++] = e.ascii;
            vga_putc(e.ascii);
        }
    }
}

bool setup_needed(void) {
    vfs_node_t *f = vfs_open("/etc/hostname", VFS_O_RDONLY);
    if (f) { vfs_close(f); return false; }
    return true;
}

void setup_run(void) {
    vga_clear();
    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    for (int i = 0; LOGO[i]; i++) {
        kprintf("%s\n", LOGO[i]);
    }
    vga_set_color(VGA_WHITE, VGA_BLACK);
    kprintf("\n          Welcome to Aerogel OS — First Boot Setup\n");
    vga_set_color(VGA_LIGHT_GRAY, VGA_BLACK);
    kprintf("--------------------------------------------------------\n\n");

    vga_set_color(VGA_YELLOW, VGA_BLACK);
    kprintf("Hostname: ");
    vga_set_color(VGA_WHITE, VGA_BLACK);
    readln();
    if (readln_len > 0) {
        strncpy(hostname, readln_buf, 63);
    } else {
        strncpy(hostname, "aerogel", 63);
    }

    vga_set_color(VGA_LIGHT_GRAY, VGA_BLACK);
    kprintf("\nSetting up filesystem...\n");

    vfs_node_t *root_dir = vfs_open("/", VFS_O_RDONLY);
    if (root_dir) {
        vfs_node_t *etc = vfs_finddir(root_dir, "etc");
        if (!etc) etc = tmpfs_mkdir(root_dir, "etc");
        tmpfs_mkdir(root_dir, "home");
        tmpfs_mkdir(root_dir, "proc");

        u32 hn_len = (u32)strlen(hostname);
        tmpfs_mkfile(etc, "hostname", (const u8 *)hostname, hn_len);
        tmpfs_mkfile(etc, "os-release",
            (const u8 *)"NAME=AerogelOS\nVERSION=0.1.0\n", 30);
    }

    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    kprintf("\nSetup complete. Hostname: %s\n", hostname);
    vga_set_color(VGA_LIGHT_GRAY, VGA_BLACK);
    kprintf("Press Enter to continue...\n");
    readln();
    vga_clear();
}
