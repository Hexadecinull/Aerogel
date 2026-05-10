#include <kernel.h>
#include <types.h>
#include <kprintf.h>
#include <gdt.h>
#include <isr.h>
#include <serial.h>
#include <pit.h>
#include <pmm.h>
#include <vmm.h>
#include <heap.h>
#include <vga_text.h>
#include <framebuffer.h>
#include <ps2_keyboard.h>
#include <ps2_mouse.h>
#include <ata.h>
#include <pci.h>
#include <xhci.h>
#include <bluetooth.h>
#include <rtl8139.h>
#include <vfs.h>
#include <fat32.h>
#include <tmpfs.h>
#include <sched.h>
#include <thread.h>
#include <gui.h>
#include <net.h>
#include <shell.h>
#include <setup.h>
#include <string.h>

#define HEAP_START 0x400000UL
#define HEAP_SIZE  (8 * 1024 * 1024UL)

static u8  ata_drv   = 0;
static bool use_fb   = false;
static gui_window_t *term_win = NULL;

static int ata_read_cb(void *ctx, u32 lba, void *buf) {
    return ata_read(*(u8 *)ctx, lba, 1, buf);
}

static void pit_handler(isr_frame_t *frame) {
    (void)frame;
    sched_tick();
}

static void kprint_fb(const char *s) {
    serial_puts(s);
    if (use_fb && term_win) gui_window_print(term_win, s, COLOR_BLACK);
    else vga_puts(s);
}

static void kprint_vga(const char *s) { serial_puts(s); vga_puts(s); }

void kmain(void) {
    serial_init();
    vga_init();
    kprintf_set_backend(kprint_vga);

    gdt_init();
    isr_init();
    isr_register(32, pit_handler);
    pit_init(100);
    __asm__ volatile("sti");

    e820_entry_t *e820_map = (e820_entry_t *)0x500;
    u32 e820_count;
    __asm__ volatile("movl 0x4FC, %0" : "=r"(e820_count));
    pmm_init(e820_map, e820_count);
    vmm_init();
    heap_init(HEAP_START, HEAP_SIZE);

    keyboard_init();
    mouse_init();

    vfs_init();
    vfs_node_t *root = tmpfs_create("root");
    vfs_mount("/", root);
    tmpfs_mkdir(root, "dev");
    tmpfs_mkdir(root, "tmp");
    tmpfs_mkdir(root, "etc");
    tmpfs_mkdir(root, "boot");
    tmpfs_mkfile(root, "version",
                 (const u8 *)"Aerogel OS v0.1.0\n", 18);

    pci_init();
    xhci_init();
    bluetooth_detect();

    net_init();
    net_bring_up();

    if (ata_init() > 0) {
        vfs_node_t *fs = fat32_mount(2048, ata_read_cb, &ata_drv);
        if (fs) vfs_mount("/data", fs);
    }

    use_fb = fb_init();
    if (use_fb) {
        gui_init();
        term_win = gui_create_window("Terminal", 20, 30,
                                     fb_width() - 40, fb_height() - 80);
        kprintf_set_backend(kprint_fb);
        kprintf("Aerogel OS v0.1.0\n");
    }

    sched_init();
    if (setup_needed()) setup_run();

    thread_t *shell = thread_create("shell", shell_run, NULL);
    sched_add(shell);
    sched_yield();

    for (;;) {
        net_poll();
        __asm__ volatile("sti; hlt");
    }
}
