#include "xhci.h"
#include <pci.h>
#include <pmm.h>
#include <string.h>
#include <kprintf.h>

#define XHCI_CLASS    0x0C
#define XHCI_SUBCLASS 0x03
#define XHCI_PROGIF   0x30

#define XHCI_CAPLENGTH  0x00
#define XHCI_HCSPARAMS1 0x04
#define XHCI_HCCPARAMS1 0x10
#define XHCI_USBCMD     0x00
#define XHCI_USBSTS     0x04
#define XHCI_DNCTRL     0x14
#define XHCI_DCBAAP     0x30
#define XHCI_CONFIG      0x38
#define XHCI_PORTSC_BASE 0x400

static bool   found    = false;
static uptr   mmio     = 0;
static u8     cap_len  = 0;
static u32    n_ports  = 0;

static volatile u32 *cap_reg(u32 off) { return (volatile u32 *)(mmio + off); }
static volatile u32 *op_reg(u32 off)  { return (volatile u32 *)(mmio + cap_len + off); }

static void xhci_reset(void) {
    volatile u32 *cmd = op_reg(XHCI_USBCMD);
    *cmd &= ~1u;
    for (int i = 0; i < 100000; i++) if (*op_reg(XHCI_USBSTS) & 1) break;

    *cmd |= (1u << 1);
    for (int i = 0; i < 100000; i++) if (!(*cmd & (1u << 1))) break;
    for (int i = 0; i < 100000; i++) if (!(*op_reg(XHCI_USBSTS) & (1u << 11))) break;
}

bool xhci_init(void) {
    const pci_dev_t *d = pci_find_class(XHCI_CLASS, XHCI_SUBCLASS, XHCI_PROGIF);
    if (!d) { kprintf("[xHCI] Not found\n"); return false; }

    mmio = d->bar[0] & ~0xFUL;
    if (!mmio) { kprintf("[xHCI] Invalid MMIO\n"); return false; }

    cap_len = (u8)(*cap_reg(XHCI_CAPLENGTH) & 0xFF);
    u32 hcs1 = *cap_reg(XHCI_HCSPARAMS1);
    n_ports  = (hcs1 >> 24) & 0xFF;
    u32 n_slots = hcs1 & 0xFF;

    kprintf("[xHCI] PCI %02x:%02x.%x  MMIO=0x%x  cap_len=%u  ports=%u\n",
            d->bus, d->dev, d->fn, (u32)mmio, cap_len, n_ports);

    u32 pci_cmd = pci_read(d->bus, d->dev, d->fn, 0x04);
    pci_write(d->bus, d->dev, d->fn, 0x04, pci_cmd | 0x06);

    xhci_reset();
    kprintf("[xHCI] Reset complete\n");

    *op_reg(XHCI_CONFIG) = n_slots & 0xFF;
    *op_reg(XHCI_DNCTRL) = 0;

    uptr dcbaap = pmm_alloc();
    if (dcbaap) {
        memset((void *)dcbaap, 0, 4096);
        *op_reg(XHCI_DCBAAP)     = (u32)(dcbaap & 0xFFFFFFFF);
        *op_reg(XHCI_DCBAAP + 4) = 0;
    }

    *op_reg(XHCI_USBCMD) = 1u;
    for (int i = 0; i < 100000; i++) if (!(*op_reg(XHCI_USBSTS) & 1)) break;
    kprintf("[xHCI] HC started\n");

    for (u32 p = 0; p < n_ports && p < 8; p++) {
        u32 portsc = *op_reg(XHCI_PORTSC_BASE + p * 0x10);
        if (portsc & 1)
            kprintf("[xHCI] Port %u: device connected (PORTSC=0x%x)\n", p+1, portsc);
    }

    found = true;
    return true;
}

bool xhci_present(void) { return found; }
