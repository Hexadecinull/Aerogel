#include "shell.h"
#include <kprintf.h>
#include <vga_text.h>
#include <ps2_keyboard.h>
#include <vfs.h>
#include <pmm.h>
#include <pit.h>
#include <sched.h>
#include <string.h>
#include <stdlib.h>
#include <serial.h>
#include <io.h>
#include <net.h>

#define CMD_BUF 256
#define MAX_ARGS 16

static char buf[CMD_BUF];
static int  buf_len = 0;

static char *args[MAX_ARGS];
static int   argc;

static void shell_putc(char c) { vga_putc(c); serial_putc(c); }

static void readline(void) {
    buf_len = 0;
    memset(buf, 0, CMD_BUF);
    for (;;) {
        key_event_t e;
        if (!keyboard_poll(&e)) { sched_yield(); continue; }
        if (!e.pressed) continue;

        if (e.ascii == '\n' || e.ascii == '\r') {
            shell_putc('\n');
            buf[buf_len] = 0;
            return;
        }
        if (e.ascii == '\b' && buf_len > 0) {
            buf_len--;
            shell_putc('\b');
            continue;
        }
        if (e.ascii && buf_len < CMD_BUF - 1) {
            buf[buf_len++] = e.ascii;
            shell_putc(e.ascii);
        }
    }
}

static void parse_args(void) {
    argc = 0;
    char *p = buf;
    while (*p == ' ') p++;
    while (*p && argc < MAX_ARGS) {
        args[argc++] = p;
        while (*p && *p != ' ') p++;
        if (*p) { *p++ = 0; while (*p == ' ') p++; }
    }
}

static void cmd_help(void) {
    kprintf("Commands: help ls cat echo clear ps mem uptime reboot ifconfig netpoll\n");
}

static void cmd_ls(const char *path) {
    if (!path) path = "/";
    vfs_node_t *dir = vfs_open(path, VFS_O_RDONLY);
    if (!dir) { kprintf("ls: no such directory: %s\n", path); return; }
    for (u32 i = 0; ; i++) {
        vfs_node_t *e = vfs_readdir(dir, i);
        if (!e) break;
        vga_set_color((e->flags & VFS_DIR) ? VGA_LIGHT_BLUE : VGA_LIGHT_GRAY, VGA_BLACK);
        kprintf("  %s%s\n", e->name, (e->flags & VFS_DIR) ? "/" : "");
    }
    vga_set_color(VGA_LIGHT_GRAY, VGA_BLACK);
    vfs_close(dir);
}

static void cmd_cat(const char *path) {
    if (!path) { kprintf("usage: cat <path>\n"); return; }
    vfs_node_t *f = vfs_open(path, VFS_O_RDONLY);
    if (!f || !(f->flags & VFS_FILE)) { kprintf("cat: not found: %s\n", path); return; }
    u8 chunk[64];
    u32 off = 0;
    u32 n;
    while ((n = vfs_read(f, (u64)off, sizeof(chunk) - 1, chunk)) > 0) {
        chunk[n] = 0;
        kprintf("%s", (char *)chunk);
        off += n;
    }
    vfs_close(f);
}

static void cmd_ps(void) {
    kprintf("  ID  NAME             STATE\n");
    kprintf("  --  ---------------  --------\n");
    thread_t *cur = thread_current();
    if (cur) kprintf("  %-3u %-16s RUNNING\n", cur->id, cur->name);
    kprintf("  (run-queue threads omitted in Phase 7)\n");
}

static void cmd_mem(void) {
    u32 total = (u32)(pmm_total()     >> 20);
    u32 used  = (u32)(pmm_used()      >> 20);
    u32 free  = (u32)(pmm_available() >> 20);
    kprintf("  Total : %u MiB\n", total);
    kprintf("  Used  : %u MiB\n", used);
    kprintf("  Free  : %u MiB\n", free);
}

static void cmd_uptime(void) {
    u32 secs = (u32)pit_ticks() / 100u;
    kprintf("  Uptime: %u min %u sec\n",
            secs / 60u, secs % 60u);
}

static void cmd_reboot(void) {
    kprintf("Rebooting...\n");
    outb(0x64, 0xFE);
    for (;;) __asm__ volatile("hlt");
}

static void dispatch(void) {
    if (!argc) return;
    const char *cmd = args[0];

    if (!strcmp(cmd, "help"))   { cmd_help(); }
    else if (!strcmp(cmd, "ls"))     { cmd_ls(argc > 1 ? args[1] : NULL); }
    else if (!strcmp(cmd, "cat"))    { cmd_cat(argc > 1 ? args[1] : NULL); }
    else if (!strcmp(cmd, "echo"))   {
        for (int i = 1; i < argc; i++) kprintf("%s%s", args[i], i<argc-1?" ":"");
        kprintf("\n");
    }
    else if (!strcmp(cmd, "clear"))  { vga_clear(); }
    else if (!strcmp(cmd, "ps"))     { cmd_ps(); }
    else if (!strcmp(cmd, "mem"))    { cmd_mem(); }
    else if (!strcmp(cmd, "uptime")) { cmd_uptime(); }
    else if (!strcmp(cmd, "reboot")) { cmd_reboot(); }
    else if (!strcmp(cmd, "ifconfig")) {
        if (net_up()) {
            u8 mac[6]; net_get_mac(mac);
            u32 ip = net_our_ip();
            kprintf("  eth0  MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
                    mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
            kprintf("        IP:  %u.%u.%u.%u\n",
                    (ip>>24)&0xFF,(ip>>16)&0xFF,(ip>>8)&0xFF,ip&0xFF);
        } else {
            kprintf("  No network interface\n");
        }
    }
    else if (!strcmp(cmd, "netpoll")) {
        net_poll();
        kprintf("Network polled.\n");
    }
    else { kprintf("%s: command not found\n", cmd); }
}

void shell_run(void *arg) {
    (void)arg;
    vga_set_color(VGA_LIGHT_GRAY, VGA_BLACK);
    kprintf("\nAerogel shell. Type 'help' for commands.\n");
    for (;;) {
        vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
        kprintf("aerogel> ");
        vga_set_color(VGA_WHITE, VGA_BLACK);
        readline();
        vga_set_color(VGA_LIGHT_GRAY, VGA_BLACK);
        parse_args();
        dispatch();
    }
}
