#include "pci.h"
#include <io.h>
#include <kprintf.h>
#include <string.h>

#define PCI_CFG_ADDR 0xCF8
#define PCI_CFG_DATA 0xCFC

static pci_dev_t devices[PCI_MAX_DEVICES];
static int       ndevices = 0;

u32 pci_read(u8 bus, u8 dev, u8 fn, u8 offset) {
    u32 addr = 0x80000000u
             | ((u32)bus  << 16)
             | ((u32)dev  << 11)
             | ((u32)fn   <<  8)
             | (offset & 0xFC);
    outl(PCI_CFG_ADDR, addr);
    return inl(PCI_CFG_DATA);
}

void pci_write(u8 bus, u8 dev, u8 fn, u8 offset, u32 val) {
    u32 addr = 0x80000000u
             | ((u32)bus  << 16)
             | ((u32)dev  << 11)
             | ((u32)fn   <<  8)
             | (offset & 0xFC);
    outl(PCI_CFG_ADDR, addr);
    outl(PCI_CFG_DATA, val);
}

static void probe(u8 bus, u8 dev, u8 fn) {
    u32 id = pci_read(bus, dev, fn, 0x00);
    if ((id & 0xFFFF) == PCI_VENDOR_NONE) return;
    if (ndevices >= PCI_MAX_DEVICES) return;

    pci_dev_t *d = &devices[ndevices++];
    memset(d, 0, sizeof(*d));
    d->bus       = bus;
    d->dev       = dev;
    d->fn        = fn;
    d->vendor_id = id & 0xFFFF;
    d->device_id = id >> 16;

    u32 cc = pci_read(bus, dev, fn, 0x08);
    d->revision  = cc & 0xFF;
    d->prog_if   = (cc >> 8)  & 0xFF;
    d->subclass  = (cc >> 16) & 0xFF;
    d->class_code= (cc >> 24) & 0xFF;

    for (int b = 0; b < 6; b++)
        d->bar[b] = pci_read(bus, dev, fn, 0x10 + b*4);

    d->irq_line = (u8)(pci_read(bus, dev, fn, 0x3C) & 0xFF);
}

void pci_init(void) {
    ndevices = 0;
    for (u16 bus = 0; bus < 256; bus++) {
        for (u8 dev = 0; dev < 32; dev++) {
            u32 id = pci_read((u8)bus, dev, 0, 0x00);
            if ((id & 0xFFFF) == PCI_VENDOR_NONE) continue;

            u32 hdr = pci_read((u8)bus, dev, 0, 0x0C);
            u8  multi = (hdr >> 16) & 0x80;
            probe((u8)bus, dev, 0);

            if (multi) {
                for (u8 fn = 1; fn < 8; fn++) {
                    u32 fid = pci_read((u8)bus, dev, fn, 0x00);
                    if ((fid & 0xFFFF) != PCI_VENDOR_NONE)
                        probe((u8)bus, dev, fn);
                }
            }
        }
    }
    kprintf("[PCI] Found %d device(s)\n", ndevices);
}

int pci_count(void) { return ndevices; }
const pci_dev_t *pci_get(int i) { return (i < ndevices) ? &devices[i] : NULL; }

const pci_dev_t *pci_find_class(u8 cls, u8 sub, u8 pif) {
    for (int i = 0; i < ndevices; i++) {
        pci_dev_t *d = &devices[i];
        if (d->class_code == cls && d->subclass == sub &&
            (pif == 0xFF || d->prog_if == pif))
            return d;
    }
    return NULL;
}
